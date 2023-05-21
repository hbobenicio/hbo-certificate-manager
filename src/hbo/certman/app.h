#pragma once

#include <gtk/gtk.h>
#include <adwaita.h>

G_BEGIN_DECLS

#define HBO_CERTMAN_TYPE_APP (hbo_certman_app_get_type())
G_DECLARE_FINAL_TYPE(HboCertmanApp, hbo_certman_app, HBO_CERTMAN, APP, AdwApplication)

HboCertmanApp* hbo_certman_app_new(void);

G_END_DECLS
