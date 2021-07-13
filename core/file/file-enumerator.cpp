#include "file-enumerator.h"
#include "file.h"
#include "log/log.h"
#include <private/qobject_p.h>

#define ENUMERATOR_FILE_NUM 100

namespace graceful
{
class FileEnumeratorPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(FileEnumerator)
public:
    void init(QString uri);
    FileEnumeratorPrivate(FileEnumerator* f);
    ~FileEnumeratorPrivate();

    void cancel();
    void enumerateASync();

    static GAsyncReadyCallback enumerateAsyncCB(GFile*, GAsyncResult*, FileEnumeratorPrivate*);
    static GAsyncReadyCallback mountMountableCB(GFile*, GAsyncResult*, FileEnumeratorPrivate*);
    static GAsyncReadyCallback mountEnclosingVolumeCB(GFile*, GAsyncResult*, FileEnumeratorPrivate*);
    static GAsyncReadyCallback enumeratorNextFilesAsyncReadyCB(GFileEnumerator*, GAsyncResult*, FileEnumeratorPrivate*);

public Q_SLOTS:
    bool onError(const GError* error);

private:
    bool                        mTryAgain = false;
    bool                        mAutoDelete = false;
    bool                        mFinished = false;
    QString                     mRootFile = nullptr;
    File*                       mFile = nullptr;
    GCancellable*               mCancellable = nullptr;
    QStringList*                mChildrenList = nullptr;
    FileEnumerator*             q_ptr = nullptr;
};

FileEnumerator::FileEnumerator(QObject *parent) : QObject(parent), d_ptr(new FileEnumeratorPrivate(this))
{
    Q_D(FileEnumerator);

    connect(this, &FileEnumerator::enumerateFinished, this, [=] (bool ret) {
        if (ret) {
            d->mFinished = true;
        }
        if (d->mAutoDelete) {
            deleteLater();
        }
    });
}

FileEnumerator::~FileEnumerator()
{

}

void FileEnumerator::setAutoDelete(bool autoDelete)
{
    Q_D(FileEnumerator);

    d->mAutoDelete = autoDelete;
}

void FileEnumerator::setEnumerateDirectory(QString uri)
{
    Q_D(FileEnumerator);
    d->init(uri);
}

void FileEnumerator::enumerateAsync()
{
    Q_D(FileEnumerator);

    const GFile* file = d->mFile->getGFile();

    g_return_if_fail(file && G_IS_FILE(file));

    log_debug("start enumerate path: '%s'", d->mRootFile.toUtf8().constData());

    g_file_enumerate_children_async(const_cast<GFile*>(file), G_FILE_ATTRIBUTE_STANDARD_NAME, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, G_PRIORITY_DEFAULT, d->mCancellable, GAsyncReadyCallback(d->enumerateAsyncCB), d);
}

const QStringList FileEnumerator::getChildrenUris()
{
    Q_D(FileEnumerator);

    if (d->mFinished) {
        return *(d->mChildrenList);
    }

    return QStringList();
}

void FileEnumeratorPrivate::init(QString uri)
{
    if (mCancellable) {
        g_cancellable_cancel(mCancellable);
        g_object_unref(mCancellable);

        mCancellable = g_cancellable_new();
    }

    if (mFile) {
        delete mFile;
        mRootFile = uri;
        mFile = new File(uri);
    }

    mChildrenList->clear();
}

FileEnumeratorPrivate::FileEnumeratorPrivate(FileEnumerator* f) : QObjectPrivate(), q_ptr(f)
{
    mRootFile = "file:///";
    mFile = new File(mRootFile);
    mCancellable = g_cancellable_new();
    mChildrenList = new QStringList;
}

FileEnumeratorPrivate::~FileEnumeratorPrivate()
{
    Q_Q(FileEnumerator);

    if (nullptr != mCancellable)    g_cancellable_cancel(mCancellable);

    q->disconnect();

    if (nullptr != mFile)           mFile->deleteLater();
    if (nullptr != mCancellable)    g_object_unref(mCancellable);
    if (nullptr != mChildrenList)   delete mChildrenList;
}

GAsyncReadyCallback FileEnumeratorPrivate::enumerateAsyncCB(GFile* file, GAsyncResult* res, FileEnumeratorPrivate* fileEnum)
{
    GError*             error = nullptr;
    GFileEnumerator*    enumerator = g_file_enumerate_children_finish(file, res, &error);

    gf_return_val_if_fail(fileEnum, nullptr);

    if (error && G_IO_ERROR_CANCELLED == error->code) {
        g_error_free(error);
        Q_EMIT fileEnum->q_func()->cancelled();
        return nullptr;
    }

    if (error) {
        fileEnum->mTryAgain = fileEnum->onError(error) && !fileEnum->mTryAgain ? true : false;
        g_autofree gchar* uri = g_file_get_uri(file);
        log_error("enumerator error: %s, uri:'%s'", error->message, uri);
        g_error_free(error);
        return nullptr;
    }

    if (!enumerator) {
        Q_EMIT fileEnum->q_func()->enumerateFinished(false);
        log_error("enumerator finiished fail!");
        return nullptr;
    }

    g_file_enumerator_next_files_async(enumerator, ENUMERATOR_FILE_NUM, G_PRIORITY_DEFAULT, fileEnum->mCancellable, GAsyncReadyCallback(enumeratorNextFilesAsyncReadyCB), fileEnum);

    return nullptr;
}

GAsyncReadyCallback FileEnumeratorPrivate::enumeratorNextFilesAsyncReadyCB(GFileEnumerator* enumerator, GAsyncResult* res, FileEnumeratorPrivate* fileEnum)
{
    gf_return_val_if_fail(fileEnum, nullptr);

    GError* error = nullptr;
    GList* files = g_file_enumerator_next_files_finish(enumerator, res, &error);
    if (error) {
        if (G_IO_ERROR_CANCELLED == error->code) {
            g_error_free(error);
            fileEnum->q_func()->cancelled();
            return nullptr;
        }

        fileEnum->q_func()->errored(error, fileEnum->mFile->path(), true);
        fileEnum->q_func()->enumerateFinished(false);

        return nullptr;
    }

    // has no file
    if (!files) {
        fileEnum->q_func()->enumerateFinished(true);
        return nullptr;
    }

    GList* l = files;
    QStringList uriList;
    int fileNum = 0;
    while (l) {
        GFileInfo* info = static_cast<GFileInfo*>(l->data);
        g_autoptr(GFile) file = g_file_enumerator_get_child(enumerator, info);
        g_autofree char* uri = g_file_get_uri(file);
        uriList << uri;
        ++fileNum;
        l = l->next;
    }
    g_list_free_full(files, g_object_unref);

    *fileEnum->mChildrenList << uriList;
    Q_EMIT fileEnum->q_func()->childrenUpdate(uriList);

    if (ENUMERATOR_FILE_NUM == fileNum) {
        g_file_enumerator_next_files_async(enumerator, ENUMERATOR_FILE_NUM, G_PRIORITY_DEFAULT, fileEnum->mCancellable, GAsyncReadyCallback(enumeratorNextFilesAsyncReadyCB), fileEnum);
    } else {
        fileEnum->q_func()->enumerateFinished(true);
    }

    return nullptr;
}

bool FileEnumeratorPrivate::onError(const GError *error)
{
    Q_Q(FileEnumerator);

    switch (error->code) {
    case G_IO_ERROR_NOT_FOUND:
    case G_IO_ERROR_EXISTS: {
        GError* error = g_error_new(G_IO_ERROR, G_IO_ERROR_EXISTS, q->tr("file '%s' not found!").toUtf8().constData(), mFile->path().toUtf8().constData());
        Q_EMIT q->errored(error, mFile->uriDisplay(), true);
    }
    break;
    case G_IO_ERROR_PERMISSION_DENIED: {
        GError* error = g_error_new(G_IO_ERROR, G_IO_ERROR_PERMISSION_DENIED, q->tr("The current user does not have permission to list the directory '%s' file").toUtf8().constData(), mFile->path().toUtf8().constData());
        Q_EMIT q->errored(error, mFile->uriDisplay(), true);
    }
    break;
    default:
        Q_EMIT q->errored(g_error_copy(error), nullptr, true);
    }

    return false;
}

}
