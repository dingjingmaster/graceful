#ifndef THUMBNAILMANAGER_H
#define THUMBNAILMANAGER_H

#include "globals.h"
#include <QObject>
#include <QSize>

namespace graceful
{
class File;
class ThumbnailManagerPrivate;
class GRACEFUL_API ThumbnailManager : public QObject
{
    Q_OBJECT
public:
    static ThumbnailManager* getInstance();
    QIcon getIcon(File& file, const QSize &size = QSize());

Q_SIGNALS:

private:
    explicit ThumbnailManager(QObject *parent = nullptr);

private:
    ThumbnailManagerPrivate*                d_ptr = nullptr;
    static ThumbnailManager*                gThumbnailManager;

    Q_DISABLE_COPY(ThumbnailManager)
    Q_DECLARE_PRIVATE(ThumbnailManager)
};
}



#endif // THUMBNAILMANAGER_H
