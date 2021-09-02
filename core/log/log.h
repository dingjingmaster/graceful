/**
 * Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H

#include <time.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <gio/gio.h>

#define LOG_VERSION "0.1.0"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    va_list             ap;
    const char*         fmt;
    const char*         file;
    struct tm*          time;
    void*               udata;
    int                 line;
    int                 level;
} log_Event;

typedef void (*log_LogFn)  (log_Event *ev);
typedef void (*log_LockFn) (bool lock, void *udata);

#ifdef LOG_EMERG
#undef LOG_EMERG
#endif
#define	LOG_EMERG	0

#ifdef LOG_ALERT
#undef LOG_ALERT
#endif
#define	LOG_ALERT	1

#ifdef LOG_CRIT
#undef LOG_CRIT
#endif
#define	LOG_CRIT	2

#ifdef LOG_ERR
#undef LOG_ERR
#endif
#define	LOG_ERR		3

#ifdef LOG_WARNING
#undef LOG_WARNING
#endif
#define	LOG_WARNING	4

#ifdef LOG_NOTICE
#undef LOG_NOTICE
#endif
#define	LOG_NOTICE	5

#ifdef LOG_INFO
#undef LOG_INFO
#endif
#define	LOG_INFO	6

#ifdef LOG_DEBUG
#undef LOG_DEBUG
#endif
#define	LOG_DEBUG	7


#define log_trace(...)  log_log (LOG_TRACE,     __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...)  log_log (LOG_DEBUG,     __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)   log_log (LOG_INFO,      __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)   log_log (LOG_WARNING,   __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...)  log_log (LOG_ERR,       __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...)  log_log (LOG_FATAL,     __FILE__, __LINE__, __VA_ARGS__)

#define gf_return_if_fail(expr) \
  G_STMT_START { \
    if (G_LIKELY (expr)) \
      { } \
    else \
      { \
        log_warn (G_STRFUNC, #expr);\
        return; \
      } \
  } G_STMT_END

#define gf_return_val_if_fail(expr, val) \
  G_STMT_START { \
    if (G_LIKELY (expr)) \
      { } \
    else \
      { \
        log_warn (G_STRFUNC, #expr);\
        return (val); \
      } \
  } G_STMT_END

#define gf_return_if_reached() \
  G_STMT_START { \
        log_warn ("file %s: line %d (%s): should not be reached", \
         __FILE__, \
         __LINE__, \
         G_STRFUNC) \
    return; \
  } G_STMT_END

#define gf_return_val_if_reached(val) \
  G_STMT_START { \
    log_warn ("file %s: line %d (%s): should not be reached", \
           __FILE__, \
           __LINE__, \
           G_STRFUNC); \
    return (val); \
  } G_STMT_END



const char* log_level_string (int level);
void log_set_lock (log_LockFn fn, void *udata);
void log_set_level (int level);
void log_set_quiet (bool enable);
int log_add_callback (log_LogFn fn, void *udata, int level);
int log_add_fp (FILE *fp, int level);

void log_log(int level, const char *file, int line, const char *fmt, ...);
#ifdef __cplusplus
}
#endif
#endif
