#include <gtk/gtk.h>

#include <hbo/certman/app.h>

int main(int argc, char **argv)
{
    g_info("starting...");
    g_autoptr(HboCertmanApp) app = hbo_certman_app_new();
    return g_application_run(G_APPLICATION(app), argc, argv);
}
