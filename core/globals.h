#ifndef GLOBALS_H
#define GLOBALS_H

#include <QtGlobal>

#ifdef COMPILE_LIBGRACEFUL
#define GRACEFUL_API Q_DECL_EXPORT
#else
#define GRACEFUL_API Q_DECL_IMPORT
#endif

#define GRACEFUL_DATA_DIR                           "/usr/share"
#define GRACEFUL_RELATIVE_SHARE_TRANSLATIONS_DIR    "graceful/translations"
#define GRACEFUL_SHARE_TRANSLATIONS_DIR             "/usr/share/graceful/translations"
#define GRACEFUL_GRAPHICS_DIR                       "/usr/share/lxqt/graphics"

#define GRACEFUL_SETTINGS_DAEMON                    "org.graceful.settings.daemon"
#define GRACEFUL_SETTINGS_ICON_THEME                "icon-theme"
#endif // GLOBALS_H
