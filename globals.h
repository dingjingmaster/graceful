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

#endif // GLOBALS_H
