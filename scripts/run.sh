#!/bin/bash
set -eu -o pipefail

# G_MESSAGES_PREFIXED
#
# A list of log levels for which messages should be prefixed by the program name and PID of the application.
# The default is to prefix everything except G_LOG_LEVEL_MESSAGE and G_LOG_LEVEL_INFO.
# The possible values are error, warning, critical, message, info and debug.
# You can also use the special values all and help.
# This environment variable only affects the default log handler, g_log_default_handler().
#
# G_MESSAGES_DEBUG
#
# A space-separated list of log domains for which informational and debug messages should be printed.
# By default, these messages are not printed. You can also use the special value all.
# This environment variable only affects the default log handler, g_log_default_handler().
#
# G_DEBUG

# This environment variable can be set to a list of debug options,
# which cause GLib to print out different types of debugging information.
#
# - fatal-warnings
#   Causes GLib to abort the program at the first call to g_warning() or g_critical().
#
# - fatal-criticals
#   Causes GLib to abort the program at the first call to g_critical().
#
# - gc-friendly
#   Newly allocated memory that isn't directly initialized, as well as memory being freed will be reset to 0.
#   The point here is to allow memory checkers and similar programs that use
#   Boehm GC alike algorithms to produce more accurate results.
#
# - resident-modules
#   All modules loaded by GModule will be made resident.
#   This can be useful for tracking memory leaks in modules which are later unloaded;
#   but it can also hide bugs where code is accessed after the module would have normally been unloaded.
#
# - bind-now-modules
#   All modules loaded by GModule will bind their symbols at load time, even when the code uses %G_MODULE_BIND_LAZY.

export G_MESSAGES_DEBUG=all
export GTK_DEBUG=interactive
./build/hbo-certificate-manager

# echo 1 | ./scripts/build.sh && G_MESSAGES_DEBUG=all GTK_DEBUG=interactive ./bin/hbo-certificate-manager
