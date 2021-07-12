#ifndef GSCREEN_H
#define GSCREEN_H

#include <QRect>
#include <QList>
#include <QSize>
#include <QPoint>
#include <QObject>
#include <QScreen>
#include <QModelIndex>

class QScreen;

namespace graceful
{

class IconView;

class GScreen : public QObject
{
    Q_OBJECT
public:
    explicit GScreen(QScreen *screen, QSize gridSize, QObject *parent = nullptr);

    bool isValidScreen();

    QRect getGeometry() const;
    QScreen* getScreen() const;

    QStringList getAllItemsOnScreen();
    QStringList getItemsOutOfScreen();
    QStringList getItemsVisibleOnScreen();
    QStringList getItemsOverrideOnScreen();
    QStringList getItemsMetaGridPosOutOfScreen();
    QStringList getItemMetaGridPosVisibleOnScreen();

    QPoint getGridCenterPoint(QPoint& pos);

    QList<QPair<QString, QPoint>> getItemsAndPosAll();
    QList<QPair<QString, QPoint>> getItemsAndPosOutOfScreen();

    bool posIsOnScreen(const QPoint& pos);
    QPoint itemGridPos(const QString &uri);
    bool isItemOutOfGrid(const QString &uri);
    bool uriIsOnScreen(const QString& uri) const;

    QPoint putIconOnScreen(const QString uri, QPoint start=QPoint(), bool force=false);
    QStringList putIconsOnScreen (const QStringList uris, bool force=false);

    void makeAllItemsGridPosInvalid();
    void makeItemGridPosInvalid(const QString& uri);
    void makeItemMetaPosInvalid(const QString& uri);

    bool setItemWithGlobalPos(const QString & uri, const QPoint& pos);
    bool saveItemWithGlobalPos(const QString& uri, const QPoint& pos);

    QPoint getItemGlobalPosition(const QString &uri);
    QString getItemFromGlobalPosition(const QPoint &pos);
    QPoint getItemMetaInfoGridPos(const QString &uri);

    void refresh();

Q_SIGNALS:
    bool iconPositionChanged(const QString& url);
    void screenVisibleChanged(bool visible);

public Q_SLOTS:
    void swapScreen(GScreen& screen);
    void rebindScreen(QScreen *screen);
    void onScreenGeometryChanged(const QRect &geometry);
    void onScreenGridSizeChanged(const QSize &gridSize);
    void setPanelMargins(const QMargins &margins);

protected:
    void recalculateGrid();

    IconView *getView();
    QString getIndexUri(const QModelIndex &index);

private:
    void clearItems();

    int maxRow() const;
    int maxColumn() const;

    bool iconIsConflict(QPoint point);

    QPoint getMetaPos (const QString& uri);
    bool saveMetaPos (const QString& uri, const QPoint& pos);

    bool setItemGridPos(const QString &uri, const QPoint &pos);

    QPoint getItemRelatedPosition(const QString &uri);
    QString getItemFromRelatedPosition(const QPoint &pos);

    QPoint coordinateGlobal2Local(const QPoint& pos) const;
    QPoint coordinateLocal2Global(const QPoint& pos) const;

    bool posAvailable(QPoint& p) const;

    QPoint placeItem(const QString &uri, QPoint lastPos=QPoint(), bool force=false);

private:
    int                             mMaxRow = 0;
    int                             mMaxColumn = 0;
    QRect                           mGeometry;
    QSize                           mGridSize;
    QMargins                        mPanelMargins;

    QHash<QString, QPoint>          mItems;
    QHash<QString, QPoint>          mItemsMetaPoses;

    QScreen*                        mScreen = nullptr;
};

}

#endif // SCREEN_H
