#ifndef FILEENUMERATOR_H
#define FILEENUMERATOR_H

#include "globals.h"
#include <QObject>

#include <gio/gio.h>

namespace graceful
{
class FileEnumeratorPrivate;
class GRACEFUL_API FileEnumerator : public QObject
{
    Q_OBJECT
public:
    explicit FileEnumerator(QObject *parent = nullptr);
    ~FileEnumerator();

    void setAutoDelete(bool autoDelete=true);
    void setEnumerateDirectory(QString uri);

    void enumerateAsync();
    const QStringList getChildrenUris();

Q_SIGNALS:
    void errored(const GError* err=nullptr, const QString& targetUri=nullptr, bool critical=false);
    void childrenUpdate(const QStringList& uriList);
    void enumerateFinished(bool successed=false);
    void cancelled();

private:
    FileEnumeratorPrivate*      d_ptr;
    Q_DISABLE_COPY(FileEnumerator)
    Q_DECLARE_PRIVATE(FileEnumerator)
};
}



#endif // FILEENUMERATOR_H
