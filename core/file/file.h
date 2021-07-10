#ifndef FILE_H
#define FILE_H

#include <QObject>

#include <gio/gio.h>

namespace graceful
{
class File : public QObject
{
    Q_OBJECT
public:
    explicit File (QString uri, QObject *parent = nullptr);

Q_SIGNALS:

private:
    GFile*              mFile;

};
}

#endif // FILE_H
