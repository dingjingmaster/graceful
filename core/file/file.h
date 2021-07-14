#ifndef FILE_H
#define FILE_H

#include <QObject>
#include "globals.h"
#include <gio/gio.h>

namespace graceful
{
class FilePrivate;

class GRACEFUL_API File : public QObject
{
    Q_OBJECT
public:
    explicit File(QString uri, QObject* parent = nullptr);
    ~File();

    QString uri();
    QString path();
    QString schema();
    QString fileName();
    QString uriDisplay();
    QString getContentType();

    QIcon icon();
    int getMIMEType();

    bool isDir();
    bool isRegularFile();

    bool isImage();
    bool isValid();
    bool isVirtual();                       // FIXME://

    bool canRead();
    bool canWrite();
    bool canExecute();
    bool canDelete();
    bool canTrash();
    bool canRename();



    const GFile* getGFile();
    const GFileInfo* getGFileStandardInfo();

Q_SIGNALS:

private:
    FilePrivate*                d_ptr = nullptr;
    Q_DISABLE_COPY(File)
    Q_DECLARE_PRIVATE(File)
};
}

#endif // FILE_H
