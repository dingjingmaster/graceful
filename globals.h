#ifndef GLOBALS_H
#define GLOBALS_H

#include <QtGlobal>

#ifdef COMPILE_LIBGRACEFUL
#define GRACEFUL_API Q_DECL_EXPORT
#else
#define GRACEFUL_API Q_DECL_IMPORT
#endif


#endif // GLOBALS_H
