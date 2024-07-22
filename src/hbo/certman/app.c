#include "app.h"

#include <stdio.h>
#include <openssl/ssl.h>

#include "main-window.h"
#include "tls/client.h"

/**
 * Global initialization of OpenSSL
 */
static void app_init_openssl(void);

static void on_app_activate(HboCertmanApp* app, gpointer data);
static void on_app_startup(HboCertmanApp* app);

static void on_action_app_quit(GSimpleAction* action, GVariant* param, gpointer app);
static void on_action_app_certs_load_from_server(GSimpleAction* action, GVariant* param, gpointer app);

static GActionEntry app_action_entries[] = {
    (GActionEntry) {
        .name = "hbo-certificate-manager.quit",
        .activate = on_action_app_quit,
        .parameter_type = NULL,
        .state = NULL,
        .change_state = NULL,
        .padding = {0},
    },
    (GActionEntry) {
        .name = "hbo-certificate-manager.certs.load-from-server",
        .activate = on_action_app_certs_load_from_server,
        .parameter_type = NULL,
        .state = NULL,
        .change_state = NULL,
        .padding = { 0 },
    },
};

struct _HboCertmanApp {
    AdwApplication parent;
};
G_DEFINE_FINAL_TYPE(HboCertmanApp, hbo_certman_app, ADW_TYPE_APPLICATION)

static void hbo_certman_app_class_init(HboCertmanAppClass* clazz)
{
    (void) clazz;
    g_debug("HboCertmanApp: class init");
}

static void hbo_certman_app_init(HboCertmanApp* app)
{
    (void) app;
    g_debug("HboCertmanApp: init");

    g_signal_connect(app, "activate", G_CALLBACK(on_app_activate), NULL);
    g_signal_connect(app, "startup",  G_CALLBACK(on_app_startup),  NULL);

    app_init_openssl();
}

HboCertmanApp* hbo_certman_app_new(void)
{
    g_debug("HboCertmanApp: new");

    HboCertmanApp* app = g_object_new(HBO_CERTMAN_TYPE_APP,
        "application-id", "br.com.hbobenicio.hbo-certificate-manager",
        "flags", G_APPLICATION_FLAGS_NONE,
        NULL
    );

    return app;
}

static void on_app_activate(HboCertmanApp* app, gpointer data)
{
    (void) data;
    g_debug("HboCertmanApp: activating application");

    HboCertmanMainWindow* window = hbo_certman_main_window_new();
    gtk_application_add_window(GTK_APPLICATION(app), GTK_WINDOW(window));

    gtk_widget_show(GTK_WIDGET(window));
}

static void on_app_startup(HboCertmanApp* app)
{
    g_action_map_add_action_entries(G_ACTION_MAP(app), app_action_entries, G_N_ELEMENTS(app_action_entries), app);
}

static void on_action_app_quit(GSimpleAction* action, GVariant* param, gpointer app)
{
    (void) action;
    (void) param;
    (void) app;
    g_info("HboCertmanApp: quitting application");
    g_application_quit(G_APPLICATION(app));
}

static void on_action_app_certs_load_from_server(GSimpleAction* action, GVariant* param, gpointer app)
{
    (void) action;
    (void) param;
    (void) app;

    g_info("HboCertmanApp: loading certificates from server");

    //TODO get the application, then the mainwindow, then the statuspage, then present a new ui for user input
    HboCertmanApp* hbo_app = HBO_CERTMAN_APP(app);

    // TODO get host/port from user
    const char* address = "www.google.com:443";
    const char* hostname = "www.google.com";
    const char* output_ca_pem_path = "/tmp/ca.fullchain.crt";
    const char* output_ca_jks_path = "/tmp/ca.jks";

    // TODO spawn a new thread for the connection and stuff and wait for it to finish
    // TODO handle reexecutions and prevent button actions while waiting for it to complete

    struct hbo_certman_tls_client tls_client;

    hbo_certman_tls_client_init(&tls_client);
    hbo_certman_tls_client_connect(&tls_client, address, hostname);
    hbo_certman_tls_client_save_peer_cert_chain(&tls_client, output_ca_pem_path, output_ca_jks_path);
    
    //TODO Alert informing the user the success of the operation

    hbo_certman_tls_client_free(&tls_client);
}

static void app_init_openssl(void)
{
    // ERR_load_BIO_strings() is deprecated since OpenSSL 3
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
}
