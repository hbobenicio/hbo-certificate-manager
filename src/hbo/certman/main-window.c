#include "main-window.h"

#include <stdio.h>

#include <openssl/ssl.h>

struct _HboCertmanMainWindow {
    AdwApplicationWindow parent;
    GtkStatusbar* status_bar;
    guint status_bar_ctx;
    GMenuModel* menu;
    GMenuItem* menu_certificate_load_from_server_item;
    GMenuItem* menu_general_quit_item;
};

G_DEFINE_FINAL_TYPE(HboCertmanMainWindow, hbo_certman_main_window, ADW_TYPE_APPLICATION_WINDOW)

static void hbo_certman_main_window_class_init(HboCertmanMainWindowClass* clazz)
{
    g_debug("HboCertmanMainWindow: class init");

    GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(clazz);

    // Template UI bindings
    gtk_widget_class_set_template_from_resource(widget_class, "/br/com/hbobenicio/hbo-certman/src/hbo/certman/main-window.ui");
    gtk_widget_class_bind_template_child(widget_class, HboCertmanMainWindow, menu);
    gtk_widget_class_bind_template_child(widget_class, HboCertmanMainWindow, status_bar);
}

static void hbo_certman_main_window_init(HboCertmanMainWindow* self)
{
    g_debug("HboCertmanMainWindow: init");

    gtk_widget_init_template(GTK_WIDGET(self));
    // g_signal_connect(self->menu_certificate_load_from_server_item, "activate", G_CALLBACK(on_menu_certificate_load_from_server_activate), NULL);

    char openssl_version_str[64] = {0};
    const char prefix[] = "OpenSSL Version: ";
    strncat(openssl_version_str, prefix, sizeof(openssl_version_str));
    strncat(openssl_version_str, OPENSSL_VERSION_STR, sizeof(openssl_version_str) - sizeof(prefix));

    self->status_bar_ctx = gtk_statusbar_get_context_id(self->status_bar, __FILE__);
    gtk_statusbar_push(self->status_bar, self->status_bar_ctx, openssl_version_str);
    
    gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(self), true);
}

HboCertmanMainWindow* hbo_certman_main_window_new(void)
{
    return g_object_new(HBO_CERTMAN_TYPE_MAIN_WINDOW, NULL);
}
