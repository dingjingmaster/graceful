#ifndef GLOBALS_H
#define GLOBALS_H

#include <QtGlobal>
#include <glib.h>

#ifdef COMPILE_LIBGRACEFUL
#define GRACEFUL_API Q_DECL_EXPORT
#include <syslog.h>
#define log_debug(...)  syslog (LOG_DEBUG, "graceful", __FILE__, __LINE__, __VA_ARGS__)
#else
#define GRACEFUL_API Q_DECL_IMPORT
#define log_debug(...)
#endif


#ifdef g_return_if_fail
#undef g_return_if_fail
#endif
#define g_return_if_fail(expr) \
  G_STMT_START { \
    if (G_LIKELY (expr)) \
      { } \
    else \
      { \
        log_debug (G_STRFUNC, #expr);\
        return; \
      } \
  } G_STMT_END

#ifdef g_return_val_if_fail
#undef g_return_val_if_fail
#endif
#define g_return_val_if_fail(expr, val) \
  G_STMT_START { \
    if (G_LIKELY (expr)) \
      { } \
    else \
      { \
        log_debug (G_STRFUNC, #expr);\
        return (val); \
      } \
  } G_STMT_END

#ifdef g_return_if_reached
#undef g_return_if_reached
#endif
#define g_return_if_reached() \
  G_STMT_START { \
        log_debug ("file %s: line %d (%s): should not be reached", G_STRFUNC) \
    return; \
  } G_STMT_END

#ifdef g_return_val_if_reached
#undef g_return_val_if_reached
#endif
#define g_return_val_if_reached(val) \
  G_STMT_START { \
    log_debug ("file %s: line %d (%s): should not be reached", G_STRFUNC); \
    return (val); \
  } G_STMT_END









// another file

#define GRACEFUL_DATA_DIR                           "/usr/share"
#define GRACEFUL_RELATIVE_SHARE_TRANSLATIONS_DIR    "graceful/translations"
#define GRACEFUL_SHARE_TRANSLATIONS_DIR             "/usr/share/graceful/translations"
#define GRACEFUL_GRAPHICS_DIR                       "/usr/share/lxqt/graphics"

#define GRACEFUL_SETTINGS_DAEMON                    "org.graceful.settings.daemon"
#define GRACEFUL_SETTINGS_ICON_THEME                "icon-theme"
#endif // GLOBALS_H
