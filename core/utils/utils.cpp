#include "utils.h"

QString graceful::Utils::urlEncode(const QString &url)
{
    QString decodeUrl = urlDecode(url);

    if (!decodeUrl.isEmpty()) {
        g_autofree gchar* encodeUrl = g_uri_escape_string (decodeUrl.toUtf8().constData(), ":/", true);
        return encodeUrl;
    }

    g_autofree gchar* encodeUrl = g_uri_escape_string (url.toUtf8().constData(), ":/", true);

    return encodeUrl;
}

QString graceful::Utils::urlDecode(const QString &url)
{
    g_autofree gchar* decodeUrl = g_uri_unescape_string(url.toUtf8(), ":/");
    if (!decodeUrl) {
        return url;
    }

    return decodeUrl;
}
