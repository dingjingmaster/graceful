#include <QDebug>
#include <QScreen>

#include "gscreen.h"
#include "icon-view.h"

#include <gio/gio.h>

namespace graceful
{

#define ICONVIEW_PADDING            5
#define INVALID_POS                 QPoint(-1, -1)

GScreen::GScreen(QScreen *screen, QSize gridSize, QObject *parent) : QObject(parent)
{
    if (!screen) {
        return;
    }

    mScreen = screen;
    mGeometry = screen->geometry();
    mGeometry.adjust(mPanelMargins.left(), mPanelMargins.top(), -mPanelMargins.right(), -mPanelMargins.bottom());
    mGridSize = gridSize;
    recalculateGrid();
    connect(screen, &QScreen::geometryChanged, this, &GScreen::onScreenGeometryChanged);
    connect(screen, &QScreen::destroyed, this, [=] () {
        mScreen = nullptr;
        Q_EMIT screenVisibleChanged(false);
    });

    connect(this, &GScreen::screenVisibleChanged, this, [=] (bool del) {

    });
}

bool GScreen::isValidScreen()
{
    return mScreen;
}

int GScreen::maxRow() const
{
    return mMaxRow;
}

int GScreen::maxColumn() const
{
    return mMaxColumn;
}

bool GScreen::iconIsConflict(QPoint pos)
{
    auto its = mItems.values();

    if (!its.contains(pos)) {
        return false;
    }

    return true;
}

bool GScreen::posIsOnScreen(const QPoint &pos)
{
    if (!isValidScreen()) return false;

    return mScreen->geometry().contains(pos);
}

void GScreen::onScreenGeometryChanged(const QRect &geometry)
{
    if (!geometry.isEmpty()) {
        mGeometry = geometry;
        mGeometry.adjust(mPanelMargins.left(), mPanelMargins.top(), -mPanelMargins.right(), -mPanelMargins.bottom());
        recalculateGrid();

        refresh();
        auto view = getView();
        if (view) view->handleScreenChanged(this);
    }
}

void GScreen::onScreenGridSizeChanged(const QSize &gridSize)
{
    if (gridSize.isEmpty()) {
        return;
    }

    mGridSize = gridSize;
    recalculateGrid();
}

void GScreen::setPanelMargins(const QMargins &margins)
{
    mPanelMargins = margins;
    mGeometry.adjust(margins.left(), margins.top(), -margins.right(), -margins.bottom());
    recalculateGrid();

    getView()->handleScreenChanged(this);
}

void GScreen::recalculateGrid()
{
    mMaxColumn = mGeometry.width() / mGridSize.width();
    mMaxRow = mGeometry.height() / mGridSize.height();

    if (mMaxColumn > 0) {
        mMaxColumn--;
    }

    if (mMaxRow > 0) {
        mMaxRow--;
    }
}

IconView* GScreen::getView()
{
    return qobject_cast<IconView *>(parent());
}

void GScreen::swapScreen(GScreen &screen)
{
    QHash<QString, QPoint>  item = mItems;
    QHash<QString, QPoint>  itemPoss = mItemsMetaPoses;

    mItems = screen.mItems;
    mItemsMetaPoses = screen.mItemsMetaPoses;

    screen.mItems = item;
    screen.mItemsMetaPoses = itemPoss;
}

QString GScreen::getIndexUri(const QModelIndex &index)
{
    return index.data(Qt::UserRole).toString();
}

void GScreen::clearItems()
{
    mItems.clear();
}

QPoint GScreen::getMetaPos(const QString& uri)
{
    if (mItemsMetaPoses.contains(uri)) {
        return mItemsMetaPoses[uri];
    }

    return INVALID_POS;
}

QRect GScreen::getGeometry() const
{
    return mGeometry;
}

QStringList GScreen::getAllItemsOnScreen()
{
    return mItems.keys();
}

QStringList GScreen::getItemsOutOfScreen()
{
    QStringList list;

    for (auto uri : mItems.keys()) {
        auto gridPos = mItems.value(uri);
        if (gridPos.x() > mMaxColumn || gridPos.y() > mMaxRow) {
            list << uri;
        }
    }

    return list;
}

QStringList GScreen::getItemsVisibleOnScreen()
{
    if (!isValidScreen()) {
        return QStringList();
    }

    QStringList list;

    QStringList uris = mItems.keys();

    for (auto uri : uris) {
        auto gridPos = mItems.value(uri);
        if (gridPos.x() <= mMaxColumn && gridPos.y() <= mMaxRow) {
            list << uri;
        }
    }

    return list;
}

QStringList GScreen::getItemsOverrideOnScreen()
{
    QStringList list;
    QMap<QString, bool> filter;

    for (auto it = mItems.constBegin(); it != mItems.constEnd(); ++it) {
        QString uri = it.key();
        QPoint pos = it.value();
        QString posStr = QString("%1x%2").arg(pos.x()).arg(pos.y());
        if (filter.contains(posStr)) {
            list << uri;
        }
        filter[posStr] = true;
    }

    return list;
}

QPoint GScreen::getItemMetaInfoGridPos(const QString &uri)
{
    QPoint poss = INVALID_POS;

    QPoint pos = mItemsMetaPoses.value(uri, INVALID_POS);
    if (INVALID_POS != pos && !iconIsConflict(pos)) {
        poss = pos;
    }

    if (poss != INVALID_POS) {
        poss = coordinateLocal2Global(poss);
    }

    return poss;
}

void GScreen::refresh()
{
    if (!isValidScreen()) {
        return;
    }

    QPoint p(0, 0);
    QStringList worngIcon;

    worngIcon << getItemsOutOfScreen();
    worngIcon << getItemsOverrideOnScreen();

    for (auto uri : worngIcon) {
        makeItemGridPosInvalid(uri);
    }

    for (auto uri : worngIcon) {
        p = placeItem(uri, p);
        if (INVALID_POS == p) {
            p = QPoint(0, 0);
        }

        mItems[uri] = p;
    }
#if 0
    qDebug() << "DEBUG_REFRESH ------------------ debug refresh ---------------------------";
    for (auto it = mItems.constBegin(); it != mItems.constEnd(); ++it) {
        qDebug() << it.key() << "  --  " << coordinateLocal2Global(it.value());
    }
    qDebug() << "DEBUG_REFRESH ------------------------------------------------------------";
#endif
}

QStringList GScreen::getItemsMetaGridPosOutOfScreen()
{
    QStringList list;

    if (!mScreen)
        return mItemsMetaPoses.keys();

    for (auto uri : mItemsMetaPoses.keys()) {
        auto gridPos = mItemsMetaPoses.value(uri);
        if (gridPos.x() > mMaxColumn || gridPos.y() > mMaxRow) {
            list << uri;
        }
    }

    return list;
}

QStringList GScreen::getItemMetaGridPosVisibleOnScreen()
{
    QStringList list;

    if (mScreen) {
        for (auto uri : mItemsMetaPoses.keys()) {
            auto gridPos = mItemsMetaPoses.value(uri);
            if (gridPos.x() <= mMaxColumn && gridPos.y() <= mMaxRow && gridPos != INVALID_POS) {
                list<<uri;
            }
        }
    }

    return list;
}

QPoint GScreen::getGridCenterPoint(QPoint &pos)
{
    if (mScreen->availableGeometry().contains(pos)) {
        QPoint localPos = coordinateGlobal2Local(pos);
        QPoint globalPos = coordinateLocal2Global(localPos);

        return QPoint(globalPos.x() + mGridSize.width() / 2, globalPos.y() + mGridSize.height() / 2);
    }

    return pos;
}

QList<QPair<QString, QPoint> > GScreen::getItemsAndPosOutOfScreen()
{
    QList<QPair<QString, QPoint>> uris;

    for (auto it = mItems.constBegin(); it != mItems.constEnd(); ++it) {
        QPoint p = it.value();
        QString uri = it.key();
        if (!posAvailable(p)) {
            uris << (QPair<QString, QPoint> (uri, p));
        }
    }

    return uris;
}

QList<QPair<QString, QPoint> > GScreen::getItemsAndPosAll()
{
    QList<QPair<QString, QPoint>> uris;

    for (auto it = mItems.constBegin(); it != mItems.constEnd(); ++it) {
        QPoint p = it.value();
        QString uri = it.key();
        uris << (QPair<QString, QPoint> (uri, coordinateLocal2Global(p)));
    }

    return uris;
}

bool GScreen::uriIsOnScreen(const QString &uri) const
{
    return mItems.contains(uri);
}

QPoint GScreen::putIconOnScreen(const QString uri, QPoint start, bool force)
{
    return coordinateLocal2Global(placeItem(uri, coordinateGlobal2Local(start), force));
}

QStringList GScreen::putIconsOnScreen(const QStringList uris, bool force)
{
    QPoint p(0, 0);
    QStringList notPut;

    for (auto u : uris) {
        p = placeItem(u, p, force);
        if (INVALID_POS == p) notPut << u;
    }

    return notPut;
}

void GScreen::makeAllItemsGridPosInvalid()
{
    mItems.clear();
}

bool GScreen::saveMetaPos(const QString &uri, const QPoint &pos)
{
    if (!isValidScreen() || "" == uri) {
        return false;
    }

    QPoint posGlob = coordinateLocal2Global(pos);

    return false;
}

QScreen* GScreen::getScreen() const
{
    return mScreen;
}

QPoint GScreen::placeItem(const QString &uri, QPoint lastPos, bool force)
{
    if (!isValidScreen() || "" == uri) {
        return INVALID_POS;
    }

    if (mItems.value(uri, INVALID_POS) != INVALID_POS) {
        mItems.remove(uri);
    }

    QPoint pos = INVALID_POS;
    int x = lastPos.x();
    int y = lastPos.y();
    while (x <= mMaxColumn && y <= mMaxRow) {
        // check if there is an index in this grid pos.
        auto tmp = QPoint(x, y);
        if (mItems.key(tmp).isEmpty()) {
            pos.setX(x);
            pos.setY(y);
            mItems.insert(uri, pos);
            return pos;
        } else {
            if (y + 1 <= mMaxRow) {
                y++;
            } else {
                y = 0;
                if (x + 1 <= mMaxColumn) {
                    x++;
                } else {
                    // out of grid, break
                    break;
                }
            }
            continue;
        }
    }

    if (force) {
        mItems.insert(uri, QPoint(0, 0));
        return QPoint(0, 0);
    }

    return pos;
}

QPoint GScreen::itemGridPos(const QString &uri)
{
    if (mItems.contains(uri)) {
        QPoint rowColum = mItems[uri];
        return coordinateLocal2Global(rowColum);
    }

    return INVALID_POS;
}

void GScreen::makeItemGridPosInvalid(const QString &uri)
{
    mItems.remove(uri);

#if 0
    qDebug() << "-------------------------" << __FUNCTION__ << "-------------------------";
    for (auto u : mItems.keys()) {
        qDebug() << "uri:" << u;
    }
    qDebug() << "------------------------------------------------------------------------";
#endif
}

void GScreen::makeItemMetaPosInvalid(const QString &uri)
{
    mItemsMetaPoses.remove(uri);
}

bool GScreen::isItemOutOfGrid(const QString &uri)
{
    auto pos = mItems.value(uri);

    if (pos == INVALID_POS) {
        return true;
    } else if (pos.x() > mMaxColumn || pos.y() > mMaxRow) {
        return false;
    }

    return true;
}

QPoint GScreen::getItemRelatedPosition(const QString &uri)
{
    if (!mScreen || "" == uri) {
        return INVALID_POS;
    }

    QPoint pos = INVALID_POS;

    if (mItems.contains(uri))  pos = mItems[uri];
    if (INVALID_POS == pos)     pos = getMetaPos(uri);
    if (INVALID_POS != pos)     return pos;

    return INVALID_POS;
}

QPoint GScreen::getItemGlobalPosition(const QString &uri)
{
    if (!mScreen || "" == uri) {
        return INVALID_POS;
    }

    auto gridPos = getItemRelatedPosition(uri);
    if (gridPos == INVALID_POS) {
        return INVALID_POS;
    }

    return coordinateLocal2Global(gridPos);
}

QString GScreen::getItemFromRelatedPosition(const QPoint &pos)
{
    if (!mScreen) {
        return nullptr;
    }

    if (pos.x() <= mMaxColumn && pos.x() >= 0 && pos.y() <= mMaxRow && pos.y() >= 0) {
        return mItems.key(pos);
    } else {
        return nullptr;
    }
}

QPoint GScreen::coordinateGlobal2Local(const QPoint &pos) const
{
    QPoint posl = pos - mScreen->geometry().topLeft();

    return QPoint(posl.x() / mGridSize.width(), posl.y() / mGridSize.height());
}

QPoint GScreen::coordinateLocal2Global(const QPoint &pos) const
{
    QPoint posg(pos.x() * mGridSize.width(), pos.y() * mGridSize.height());

    g_return_val_if_fail(mScreen, posg);

    return mScreen->geometry().topLeft() + posg;
}

bool GScreen::posAvailable(QPoint &p) const
{
    if (p.x() >= mMaxRow || p.y() >= mMaxColumn) {
        return false;
    }

    return true;
}

QString GScreen::getItemFromGlobalPosition(const QPoint &pos)
{
    if (!mScreen) {
        return nullptr;
    }

    return getItemFromRelatedPosition(coordinateGlobal2Local(pos));
}

bool GScreen::setItemGridPos(const QString &uri, const QPoint &pos)
{
    auto currentGridPos = mItems.value(uri);
    if (currentGridPos == pos) {
        return true;
    }

    if (pos.x() > mMaxColumn || pos.y() > mMaxRow || pos.x() < 0 || pos.y() < 0 || iconIsConflict(pos)) {
        return false;
    }

    auto itemOnTargetPos = mItems.key(pos);
    if (itemOnTargetPos.isEmpty()) {
        mItems.insert(uri, pos);
        return true;
    } else {
        return false;
    }
}

bool GScreen::setItemWithGlobalPos(const QString &uri, const QPoint &pos)
{
    if (mScreen && mScreen->availableGeometry().contains(pos)) {
        return setItemGridPos(uri, coordinateGlobal2Local(pos));
    }

    return false;
}

bool GScreen::saveItemWithGlobalPos(const QString &uri, const QPoint &pos)
{
    if (mScreen && mScreen->geometry().contains(pos)) {
        return saveMetaPos(uri, coordinateGlobal2Local(pos));
    }

    return false;
}

void GScreen::rebindScreen(QScreen *screen)
{
    if (!screen) {
        return;
    }

    if (mScreen) {
        mScreen->disconnect(mScreen, &QScreen::geometryChanged, this, 0);
        mScreen->disconnect(mScreen, &QScreen::destroyed, this, 0);
    }

    QHash<QString, QPoint>  item = mItems;
    QHash<QString, QPoint>  itemPoss = mItemsMetaPoses;

    // FIXME://
    mGeometry.adjust(mPanelMargins.left(), mPanelMargins.top(), -mPanelMargins.right(), -mPanelMargins.bottom());
    connect(mScreen, &QScreen::geometryChanged, this, &GScreen::onScreenGeometryChanged);
    connect(mScreen, &QScreen::destroyed, this, [=](){
        mScreen = nullptr;
        Q_EMIT screenVisibleChanged(false);
    });

    Q_EMIT screenVisibleChanged(true);
}

}
