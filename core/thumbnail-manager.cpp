#include "thumbnail-manager.h"
#include "regular-file-type.h"
#include "file/file.h"
#include "log/log.h"
#include <gio/gio.h>

#include <QMap>
#include <QIcon>
#include <QImage>
#include <QDebug>

static QIcon imageToIcon (const QImage& image, const QSize& size);

graceful::ThumbnailManager* graceful::ThumbnailManager::gThumbnailManager = new ThumbnailManager;

namespace graceful
{
class ThumbnailManagerPrivate
{
public:
    ThumbnailManagerPrivate();
    ~ThumbnailManagerPrivate();

    QImage getPictureThumbnail(File& file);
    QImage getStandardThumbnail(File& file) const;


public:
    int                                     mMaxSize = 1000;
    QIcon                                   mInvalidIcon;
    QMap<QString, QImage>*                  mUriImage = nullptr;                    // The icon corresponding to the file // maybe use shared ptr
    QMap<MIMEType, QImage>*                 mTypeImage = nullptr;                   // The icon corresponding to the file type
};

ThumbnailManagerPrivate::ThumbnailManagerPrivate()
{
    mUriImage = new QMap<QString, QImage>();
    mTypeImage = new QMap<MIMEType, QImage>();
}

ThumbnailManagerPrivate::~ThumbnailManagerPrivate()
{
    if (mUriImage)                          delete mUriImage;
    if (mTypeImage)                         delete mTypeImage;
}

QImage ThumbnailManagerPrivate::getPictureThumbnail(File &file)
{
    QString uri = file.uri();

    if (mUriImage->contains(uri)) {
        return mUriImage->value(uri);
    }

    QImage img(file.path());
    if (img.size().width() > 128) {
        img = img.scaledToWidth(128, Qt::SmoothTransformation);
    }

    if (img.size().height() > 128) {
        img = img.scaledToHeight(128, Qt::SmoothTransformation);
    }

    if (mUriImage->size() >= mMaxSize) {
        mUriImage->clear();
    }

    mUriImage->insert(uri, img);

    return img;
}

QImage ThumbnailManagerPrivate::getStandardThumbnail(File &file) const
{
    QString uri = file.uri();

    if (mUriImage->contains(uri)) {
        return mUriImage->value(uri);
    }

    GFileInfo* fileInfo = const_cast<GFileInfo*>(file.getGFileStandardInfo());
    g_return_val_if_fail(fileInfo, QImage());

    GIcon* icons = g_file_info_get_symbolic_icon(fileInfo);
    g_autofree gchar* iconNames = g_icon_to_string(icons);

    log_debug("get icon name:%s", iconNames);

    QString ticonNames = iconNames;
    QStringList qiconNames = ticonNames.split(" ");
    for (auto n : qiconNames) {
        QIcon icon = QIcon::fromTheme(n);
        if (!icon.isNull()) {
            mTypeImage->insert(static_cast<MIMEType>(file.getMIMEType()), icon.pixmap(QSize(128, 128)).toImage());
        }
    }

    return mTypeImage->value(static_cast<MIMEType>(file.getMIMEType()), QImage());
}
}


graceful::ThumbnailManager *graceful::ThumbnailManager::getInstance()
{
    return gThumbnailManager;
}

// FIXME:// need more type, check return icon is valid, if not, draw a icon
QIcon graceful::ThumbnailManager::getIcon(File& file, const QSize& size)
{
    Q_D(ThumbnailManager);

    gf_return_val_if_fail(file.isValid(), d->mInvalidIcon);

    if (file.isImage()) {
        log_debug("file is image!");
        return imageToIcon(d->getPictureThumbnail(file), size);
    }

    log_debug("get standard file info's icon!");
    return imageToIcon(d->getStandardThumbnail(file), size);
}

static QIcon imageToIcon (const QImage& image, const QSize& size)
{
    QIcon icon;
    QImage img = image;

    if (size.isValid()) {
        img = img.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    icon.addPixmap(QPixmap::fromImage(img));

    return icon;
}

graceful::ThumbnailManager::ThumbnailManager(QObject *parent) : QObject(parent), d_ptr(new ThumbnailManagerPrivate)
{

}
