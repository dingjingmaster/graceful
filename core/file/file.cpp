#include "file.h"
#include "utils/utils.h"

#include <QIcon>
#include <QUrl>
#include <private/qobject_p.h>

namespace graceful
{
class FilePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(File)
public:
    explicit FilePrivate(File* f, QString uri);
    ~FilePrivate();

public:
    GFile*                              mFile;
    File*                               q_ptr = nullptr;
};

FilePrivate::FilePrivate(File* f, QString uri) : QObjectPrivate(), q_ptr(f)
{
    QString encodeUrl = Utils::urlEncode(uri);
    QUrl url(encodeUrl);

    if (!url.scheme().isNull()) {
        mFile = g_file_new_for_uri(encodeUrl.toUtf8().constData());
    } else if (uri.startsWith("/")) {
        mFile = g_file_new_for_path(encodeUrl.toUtf8().constData());
    }
}

FilePrivate::~FilePrivate()
{
    if (mFile)                              g_object_unref(mFile);
}
}


graceful::File::File(QString uri, QObject *parent) : QObject(parent), d_ptr(new FilePrivate(this, uri))
{

}

graceful::File::~File()
{

}

QString graceful::File::uri()
{
    Q_D(File);

    g_autofree char* uri = g_file_get_uri(d->mFile);

    return uri;
}

QString graceful::File::path()
{
    Q_D(File);

    g_autofree char* path = g_file_get_path(d->mFile);

    return path;
}

QString graceful::File::fileName()
{
    Q_D(File);

    g_autofree char* name = g_file_get_basename(d->mFile);

    return name;
}

QString graceful::File::uriDisplay()
{
    return Utils::urlDecode(uri());
}

QIcon graceful::File::icon()
{
    // fixme://
    return QIcon();
}

bool graceful::File::isDir()
{
    Q_D(File);

    GFileType type = g_file_query_file_type(d->mFile, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr);

    return type == G_FILE_TYPE_DIRECTORY;
}

bool graceful::File::isValid()
{
    Q_D(File);

    return g_file_query_exists(d->mFile, nullptr);
}

bool graceful::File::isVirtual()
{
    Q_D(File);

    // fixme://

    return false;
}

const GFile *graceful::File::getGFile()
{
    Q_D(File);

    return d->mFile;
}

