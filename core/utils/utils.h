#ifndef UTILS_H
#define UTILS_H

#include "globals.h"

#include <QString>

#include <gio/gio.h>

namespace graceful
{

#define BLOCKING
#define NO_BLOCKING

class GRACEFUL_API Utils
{
public:
    NO_BLOCKING static QString urlEncode(const QString& url);
    NO_BLOCKING static QString urlDecode(const QString& url);
};
}


#endif // UTILS_H
