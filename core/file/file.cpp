#include "file.h"
#include "log/log.h"
#include "utils/utils.h"
#include "regular-file-type.h"
#include "thumbnail-manager.h"

#include <QUrl>
#include <QIcon>
#include <QDebug>
#include <QRegExp>
#include <private/qobject_p.h>

namespace graceful
{
// .jpg, .jpeg, .jfif, .pjpeg, .pjp, .png
QRegExp gImageFile("(\\.jpg|\\.jpeg|\\.jfif|\\.pjpeg|\\.pjp|\\.png)$");

class FilePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(File)
public:
    explicit FilePrivate(File* f, QString uri);
    ~FilePrivate();

    void queryFileType();

public:
    GFile*                              mFile = nullptr;
    GFileInfo*                          mFileStandardInfo = nullptr;

    QString                             mUri = nullptr;

    GFileType                           mFileType = G_FILE_TYPE_UNKNOWN;
    MIMEType                            mFileMimeType = FILE_TYPE_UNKNOW;

    File*                               q_ptr = nullptr;
};

FilePrivate::FilePrivate(File* f, QString uri) : QObjectPrivate(), q_ptr(f)
{
    if (uri.split("://").size() == 2) {
        mUri = uri;
    } else if (uri.startsWith("/")) {
        mUri = "file://" + uri;
    }

    log_debug("new file by uri:%s", mUri.toUtf8().constData());

    if (!mUri.isNull()) {
        mFile = g_file_new_for_uri(Utils::urlEncode(mUri).toUtf8().constData());
        mFileStandardInfo = g_file_query_info(mFile, "standard::*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, nullptr);
    }
}

FilePrivate::~FilePrivate()
{
    if (mFile)                              g_object_unref(mFile);
    if (mFileStandardInfo)                  g_object_unref(mFileStandardInfo);
}

void FilePrivate::queryFileType()
{
    if (G_FILE_TYPE_UNKNOWN == mFileType) {
        mFileType = g_file_query_file_type(mFile, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr);
    }
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
    // FIXME:// draw xxx.xxx icon
    gf_return_val_if_fail(G_FILE(getGFile()), QIcon());

    return ThumbnailManager::getInstance()->getIcon(*this);
}

int graceful::File::getMIMEType()
{
    Q_D(File);

    return d->mFileMimeType;
}

bool graceful::File::isDir()
{
    Q_D(File);

    d->queryFileType();

    if (d->mFileType == G_FILE_TYPE_DIRECTORY) {
        d->mFileMimeType = FILE_TYPE_DIRECTORY;
    }

    return false;
}

bool graceful::File::isImage()
{
    Q_D(File);

    if (FILE_TYPE_IMAGE == d->mFileMimeType) {
        return true;
    }

    if (isRegularFile() && path().contains(gImageFile)) {
        d->mFileMimeType = FILE_TYPE_IMAGE;
        return true;
    }

    return false;
}

bool graceful::File::isRegularFile()
{
    Q_D(File);

    d->queryFileType();

    return d->mFileType == G_FILE_TYPE_REGULAR;
}

bool graceful::File::isValid()
{
    Q_D(File);

    return g_file_query_exists(d->mFile, nullptr);
}

bool graceful::File::isVirtual()
{
    Q_D(File);

    d->queryFileType();
    // fixme://

    return false;
}

const GFile* graceful::File::getGFile()
{
    Q_D(File);

    return d->mFile;
}

const GFileInfo* graceful::File::getGFileStandardInfo()
{
    Q_D(File);

    return d->mFileStandardInfo;
}

