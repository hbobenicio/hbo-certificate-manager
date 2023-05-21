#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#include <gtk/gtk.h>
#include <openssl/err.h>

#include <hbo/certman/utils/array.h>
#include <hbo/certman/utils/fs.h>

#define LOG_TAG "tls/client: "

void hbo_certman_tls_client_init(struct hbo_certman_tls_client* client)
{
    g_debug(LOG_TAG "initializing");

    // The following return values can occur:
    // - NULL: The creation of a new SSL_CTX object failed. Check the error stack to find out the reason.
    // - Pointer to an SSL_CTX object: The return value points to an allocated SSL_CTX object.
    client->ctx = SSL_CTX_new(TLS_client_method());
    assert(client->ctx != NULL && "OpenSSL Context must be created");

    // Specifies that the default locations from which CA certificates are loaded should be used.
    // There is one default directory, one default file and one default store.
    // The default CA certificates directory is called certs in the default OpenSSL directory, and this is also the default store.  Alternatively the
    // SSL_CERT_DIR environment variable can be defined to override this location.
    // The default CA certificates file is called cert.pem in the default OpenSSL directory.
    // Alternatively the SSL_CERT_FILE environment variable can be defined to override this location
    //
    // returns 1 on success or 0 on failure. A missing default location is still treated as a success
    int rc = SSL_CTX_set_default_verify_paths(client->ctx);
    assert(rc == 1);

    // sets the timeout for newly created sessions for ctx to t. The timeout value t must be given in seconds
    // returns the previously set timeout value
    SSL_CTX_set_timeout(client->ctx, 5);

    // creates a new BIO chain consisting of an SSL BIO (using ctx) followed by a connect BIO.
    // returns a valid BIO structure on success or NULL if an error occurred
    client->bio = BIO_new_ssl_connect(client->ctx);
    assert(client->bio != NULL);

    // retrieves the SSL pointer of SSL BIO b, it can then be manipulated using the standard SSL library functions
    BIO_get_ssl(client->bio, &client->ssl);
    assert(client->ssl != NULL && "TLS/SSL Session must be created");

    // adds the mode set via bit-mask in mode to ssl. Options already set before are not cleared.
    // SSL_clear_mode() removes the mode set via bit-mask in mode from ssl.
    // see `man SSL_set_mode` for details about the flags
    SSL_set_mode(client->ssl, SSL_MODE_AUTO_RETRY);
}

void hbo_certman_tls_client_free(struct hbo_certman_tls_client* client)
{
    BIO_free_all(client->bio);
    client->bio = NULL;

    // SSL* is managed by the BIO. The call above frees it
    client->ssl = NULL;

    SSL_CTX_free(client->ctx);
    client->ctx = NULL;
}

void hbo_certman_tls_client_connect(struct hbo_certman_tls_client* client, const char* address, const char* hostname)
{
    // sets the server name indication ClientHello extension to contain the value name.
    // The type of server name indication extension is set to TLSEXT_NAMETYPE_host_name (defined in RFC3546)
    // returns 1 on success, 0 in case of error
    int rc = SSL_set_tlsext_host_name(client->ssl, hostname);
    assert(rc == 1);

    // Uses the string name to set the hostname.
    // The hostname can be an IP address; if the address is an IPv6 one, it must be enclosed with brackets "[" // and "]".
    // The hostname can also include the port in the form hostname:port.
    // see BIO_parse_hostserv(3) and BIO_set_conn_port() for details
    //
    // returns 1 on success and <=0 on failure
    //
    //NOTE this may include the port too
    //NOTE Should we be using BIO_new_connect???
    //     BIO_new_connect() combines BIO_new() and BIO_set_conn_hostname() into a single call:
    //         that is it creates a new connect BIO with hostname name.
    rc = BIO_set_conn_hostname(client->bio, address);
    assert(rc == 1);

    g_info(LOG_TAG "connecting... hostname=\"%s\"", hostname);

    if (BIO_do_connect(client->bio) != 1) {
        g_error(LOG_TAG "connection failed: openssl reason: %s", ERR_reason_error_string(ERR_get_error()));
        return;
    }

    g_info(LOG_TAG "connected.");

    if (SSL_get_verify_result(client->ssl) != X509_V_OK) {
        g_error(LOG_TAG "openssl verification failed. openssl reason: %s", ERR_reason_error_string(ERR_get_error()));
        return;
    }

    g_debug(LOG_TAG "openssl verification succeeded.");
}

void hbo_certman_tls_client_save_peer_cert_chain(struct hbo_certman_tls_client* client, const char* output_pem_file_path, const char* output_jks_file_path)
{
    g_debug(LOG_TAG "getting peer certificates chain...");

    // STACK_OF(X509)* verified_cert_chain = SSL_get0_verified_chain(this->ssl);
    STACK_OF(X509)* server_cert_chain = SSL_get_peer_cert_chain(client->ssl);
    if (server_cert_chain == NULL) {
        g_error(LOG_TAG "failed to get server certificate chain: openssl reason: %s", ERR_reason_error_string(ERR_get_error()));
        return;
    }

    g_debug(LOG_TAG "opening PEM output file... path=\"%s\"", output_pem_file_path);

    BIO* output_pem_file_bio = BIO_new_file(output_pem_file_path, "wb");
    if (output_pem_file_bio == NULL) {
        g_error(LOG_TAG "failed to open output path to write server certificates chain: openssl reason: %s",
            ERR_reason_error_string(ERR_get_error()));
        return;
    }

    g_debug(LOG_TAG "writing %d certificates to PEM output file", sk_X509_num(server_cert_chain));
    for (int i = 0; i < sk_X509_num(server_cert_chain); i++) {
        X509* cert = sk_X509_value(server_cert_chain, i);
        PEM_write_bio_X509(output_pem_file_bio, cert);
    }

    BIO_free(output_pem_file_bio);

    g_info(LOG_TAG "PEM certificates written successfully. output_pem_file_path=\"%s\"", output_pem_file_path);
    
    g_debug(LOG_TAG "searching for keytool binary in the PATH environment variable...");
    
    char keytool_path[512] = {0};
    int rc = find_bin_in_path("keytool", keytool_path, ARRAY_SIZE(keytool_path));
    if (rc != 0) {
        g_error(LOG_TAG "failed to find ");
        return;
    }

    g_debug(LOG_TAG "found keytool at \"%s\"", keytool_path);
    
    //TODO call keytool child process (multiple times for each certificate)
    pid_t pid = fork();
    assert(pid >= 0);

    if (pid == 0) {
        // child process
        // keytool -import -alias foo -keystore ca.jks -file ca.pem
        //TODO bring better alias
        execl(keytool_path,
                "keytool",
                "-importcert",
                "-alias", "my-alias",
                "-keystore", output_jks_file_path,
                "-file", output_pem_file_path,
                NULL);

        // just in case... (at this point the process is not executing our code anymore)
        return;
    }

    int child_process_rc;
    rc = waitpid(pid, &child_process_rc, 0);
    assert(rc != -1);

    g_debug(LOG_TAG "child process rc=%d", child_process_rc);
}
