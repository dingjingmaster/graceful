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

    void queryAccessInfo();

public:
    GFile*                              mFile = nullptr;
    GFileInfo*                          mFileStandardInfo = nullptr;

    QString                             mUri = nullptr;
    QString                             mSchema = nullptr;

    GFileType                           mFileType = G_FILE_TYPE_UNKNOWN;
    MIMEType                            mFileMimeType = FILE_TYPE_UNKNOW;

    bool                                mQueryAccess = false;
    bool                                mCanRead = true;
    bool                                mCanWrite = true;
    bool                                mCanExecute = true;
    bool                                mCanDelete = true;
    bool                                mCanTrash = true;
    bool                                mCanRename = true;

    QString                             mContentType = nullptr;


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

    if (!mUri.isNull() && !mUri.isEmpty()) {
        mFile = g_file_new_for_uri(Utils::urlEncode(mUri).toUtf8().constData());
        mFileStandardInfo = g_file_query_info(mFile, "standard::*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, nullptr);

        QStringList ls = mUri.split("://");
        if (2 == ls.size()) {
            mSchema = ls.first();
        }
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

void FilePrivate::queryAccessInfo()
{
    gf_return_if_fail(G_IS_FILE(mFile));

    GError* error = nullptr;
    g_autoptr(GFileInfo) fileInfo = g_file_query_info(mFile, "access::*", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, nullptr, &error);
    if (error) {
        log_error("query file info access:: error: %d -- %s", error->code, error->message);
        g_error_free(error);
        return;
    }

    if (g_file_info_has_attribute(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_READ)) {
        mCanRead = g_file_info_get_attribute_boolean(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_READ);
    }

    if (g_file_info_has_attribute(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE)) {
        mCanWrite = g_file_info_get_attribute_boolean(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE);
    }

    if (g_file_info_has_attribute(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE)) {
        mCanExecute = g_file_info_get_attribute_boolean(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_EXECUTE);
    }

    if (g_file_info_has_attribute(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_DELETE)) {
        mCanDelete = g_file_info_get_attribute_boolean(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_DELETE);
    }

    if (g_file_info_has_attribute(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_TRASH)) {
        mCanTrash = g_file_info_get_attribute_boolean(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_TRASH);
    }

    if (g_file_info_has_attribute(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_RENAME)) {
        mCanRename = g_file_info_get_attribute_boolean(fileInfo, G_FILE_ATTRIBUTE_ACCESS_CAN_RENAME);
    }
    mQueryAccess = true;
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

QString graceful::File::schema()
{
    Q_D(File);
    if (!d->mSchema.isNull() && !d->mSchema.isEmpty()) {
        return d->mSchema;
    }

    return "";
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

QString graceful::File::getContentType()
{
    Q_D(File);

    if (!d->mContentType.isNull() && !d->mContentType.isEmpty()) {
        return d->mContentType;
    }

    gf_return_val_if_fail(G_IS_FILE_INFO(d->mFileStandardInfo), nullptr);

    d->mContentType = g_file_info_get_content_type(d->mFileStandardInfo);

    gf_return_val_if_fail(!d->mContentType.isNull() && !d->mContentType.isEmpty(), "");

    return d->mContentType;
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

bool graceful::File::canRead()
{
    Q_D(File);

    if (!d->mQueryAccess) {
        d->queryAccessInfo();
    }

    return d->mCanRead;
}

bool graceful::File::canWrite()
{
    Q_D(File);

    if (!d->mQueryAccess) {
        d->queryAccessInfo();
    }

    return d->mCanWrite;
}

bool graceful::File::canExecute()
{
    Q_D(File);

    if (!d->mQueryAccess) {
        d->queryAccessInfo();
    }

    return d->mCanExecute;
}

bool graceful::File::canDelete()
{
    Q_D(File);

    if (!d->mQueryAccess) {
        d->queryAccessInfo();
    }

    return d->mCanDelete;
}

bool graceful::File::canTrash()
{
    Q_D(File);

    if (!d->mQueryAccess) {
        d->queryAccessInfo();
    }

    return d->mCanTrash;
}

bool graceful::File::canRename()
{
    Q_D(File);

    if (!d->mQueryAccess) {
        d->queryAccessInfo();
    }

    return d->mCanRename;
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

