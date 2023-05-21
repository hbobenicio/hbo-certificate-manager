#pragma once

#include <openssl/bio.h>
#include <openssl/ssl.h>

struct hbo_certman_tls_client {
    SSL_CTX* ctx;
    BIO* bio;
    SSL* ssl;
};

void hbo_certman_tls_client_init(struct hbo_certman_tls_client* client);
void hbo_certman_tls_client_free(struct hbo_certman_tls_client* client);
void hbo_certman_tls_client_connect(struct hbo_certman_tls_client* client, const char* address, const char* hostname);
void hbo_certman_tls_client_save_peer_cert_chain(struct hbo_certman_tls_client* client, const char* output_pem_file_path, const char* output_jks_file_path);
