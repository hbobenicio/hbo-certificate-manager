#pragma once

#include <gtk/gtk.h>
#include <adwaita.h>

G_BEGIN_DECLS

#define HBO_CERTMAN_TYPE_MAIN_WINDOW (hbo_certman_main_window_get_type())
G_DECLARE_FINAL_TYPE(HboCertmanMainWindow, hbo_certman_main_window, HBO_CERTMAN, MAIN_WINDOW, AdwApplicationWindow)

HboCertmanMainWindow* hbo_certman_main_window_new(void);

G_END_DECLS
