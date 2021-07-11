#include <QDebug>
#include <QScreen>

#include "screen.h"
#include "desktop-view.h"

namespace graceful
{

#define ICONVIEW_PADDING            5
#define INVALID_POS                 QPoint(-1, -1)

GScreen::GScreen(QScreen *screen, QSize gridSize, QObject *parent) : QObject(parent)
{
    if (!screen) {
        qCritical()<<"invalid screen";
        return;
    }

    m_screen = screen;
    m_geometry = screen->geometry();
    m_geometry.adjust(m_panelMargins.left(), m_panelMargins.top(), -m_panelMargins.right(), -m_panelMargins.bottom());
    m_gridSize = gridSize;
    recalculateGrid();
    connect(screen, &QScreen::geometryChanged, this, &GScreen::onScreenGeometryChanged);
    connect(screen, &QScreen::destroyed, this, [=] () {
        m_screen = nullptr;
        Q_EMIT screenVisibleChanged(false);
    });

    connect(this, &GScreen::screenVisibleChanged, this, [=] (bool del) {

    });
}

bool GScreen::isValidScreen()
{
    return m_screen;
}

int GScreen::maxRow() const
{
    return m_maxRow;
}

int GScreen::maxColumn() const
{
    return m_maxColumn;
}

bool GScreen::iconIsConflict(QPoint pos)
{
    auto its = m_items.values();

    if (!its.contains(pos)) {
        return false;
    }

    return true;
}

bool GScreen::posIsOnScreen(const QPoint &pos)
{
    if (!isValidScreen()) return false;

    return m_screen->geometry().contains(pos);
}

void GScreen::onScreenGeometryChanged(const QRect &geometry)
{
    if (!geometry.isEmpty()) {
        m_geometry = geometry;
        m_geometry.adjust(m_panelMargins.left(), m_panelMargins.top(), -m_panelMargins.right(), -m_panelMargins.bottom());
        recalculateGrid();

        refresh();
        auto view = getView();
        if (view) view->handleScreenChanged(this);
    }
}

void GScreen::onScreenGridSizeChanged(const QSize &gridSize)
{
    if (gridSize.isEmpty()) {
        qCritical()<<"invalid grid size";
        return;
    }

    m_gridSize = gridSize;
    recalculateGrid();
}

void GScreen::setPanelMargins(const QMargins &margins)
{
    m_panelMargins = margins;
    m_geometry.adjust(margins.left(), margins.top(), -margins.right(), -margins.bottom());
    recalculateGrid();

    getView()->handleScreenChanged(this);
}

void GScreen::recalculateGrid()
{
    m_maxColumn = m_geometry.width() / m_gridSize.width();
    m_maxRow = m_geometry.height() / m_gridSize.height();

    if (m_maxColumn > 0) {
        m_maxColumn--;
    }

    if (m_maxRow > 0) {
        m_maxRow--;
    }
}

DesktopView* GScreen::getView()
{
    return qobject_cast<DesktopView *>(parent());
}

void GScreen::swapScreen(GScreen &screen)
{
    QHash<QString, QPoint>  item = m_items;
    QHash<QString, QPoint>  itemPoss = m_itemsMetaPoses;

    m_items = screen.m_items;
    m_itemsMetaPoses = screen.m_itemsMetaPoses;

    screen.m_items = item;
    screen.m_itemsMetaPoses = itemPoss;
}

QString GScreen::getIndexUri(const QModelIndex &index)
{
    return index.data(Qt::UserRole).toString();
}

void GScreen::clearItems()
{
    m_items.clear();
}

QPoint GScreen::getMetaPos(const QString& uri)
{
    if (m_itemsMetaPoses.contains(uri)) {
        return m_itemsMetaPoses[uri];
    }

    return INVALID_POS;
}

QRect GScreen::getGeometry() const
{
    return m_geometry;
}

QStringList GScreen::getAllItemsOnScreen()
{
    return m_items.keys();
}

QStringList GScreen::getItemsOutOfScreen()
{
    QStringList list;

    for (auto uri : m_items.keys()) {
        auto gridPos = m_items.value(uri);
        if (gridPos.x() > m_maxColumn || gridPos.y() > m_maxRow) {
            qDebug() << "屏幕外的 uri:" << uri;
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

    QStringList uris = m_items.keys();

    for (auto uri : uris) {
        auto gridPos = m_items.value(uri);
        if (gridPos.x() <= m_maxColumn && gridPos.y() <= m_maxRow) {
            qDebug() << "屏幕上可见的 uri: " << uri;
            list << uri;
        }
    }

    return list;
}

QStringList GScreen::getItemsOverrideOnScreen()
{
    QStringList list;
    QMap<QString, bool> filter;

    for (auto it = m_items.constBegin(); it != m_items.constEnd(); ++it) {
        QString uri = it.key();
        QPoint pos = it.value();
        QString posStr = QString("%1x%2").arg(pos.x()).arg(pos.y());
        if (filter.contains(posStr)) {
            list << uri;
            qDebug() << "重叠的 uri:" << uri << " -- " << posStr << " point:" << coordinateLocal2Global(pos);
        }
        filter[posStr] = true;
    }

    return list;
}

QPoint GScreen::getItemMetaInfoGridPos(const QString &uri)
{
    QPoint poss = INVALID_POS;

    QPoint pos = m_itemsMetaPoses.value(uri, INVALID_POS);
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

        m_items[uri] = p;
    }
#if 0
    qDebug() << "DEBUG_REFRESH ------------------ debug refresh ---------------------------";
    for (auto it = m_items.constBegin(); it != m_items.constEnd(); ++it) {
        qDebug() << it.key() << "  --  " << coordinateLocal2Global(it.value());
    }
    qDebug() << "DEBUG_REFRESH ------------------------------------------------------------";
#endif
}

QStringList GScreen::getItemsMetaGridPosOutOfScreen()
{
    QStringList list;

    if (!m_screen)
        return m_itemsMetaPoses.keys();

    for (auto uri : m_itemsMetaPoses.keys()) {
        auto gridPos = m_itemsMetaPoses.value(uri);
        if (gridPos.x() > m_maxColumn || gridPos.y() > m_maxRow) {
            list << uri;
        }
    }

    return list;
}

QStringList GScreen::getItemMetaGridPosVisibleOnScreen()
{
    QStringList list;

    if (m_screen) {
        for (auto uri : m_itemsMetaPoses.keys()) {
            auto gridPos = m_itemsMetaPoses.value(uri);
            if (gridPos.x() <= m_maxColumn && gridPos.y() <= m_maxRow && gridPos != INVALID_POS) {
                list<<uri;
            }
        }
    }

    return list;
}

QPoint GScreen::getGridCenterPoint(QPoint &pos)
{
    if (m_screen->availableGeometry().contains(pos)) {
        QPoint localPos = coordinateGlobal2Local(pos);
        QPoint globalPos = coordinateLocal2Global(localPos);

        return QPoint(globalPos.x() + m_gridSize.width() / 2, globalPos.y() + m_gridSize.height() / 2);
    }

    return pos;
}

QList<QPair<QString, QPoint> > GScreen::getItemsAndPosOutOfScreen()
{
    QList<QPair<QString, QPoint>> uris;

    for (auto it = m_items.constBegin(); it != m_items.constEnd(); ++it) {
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

    for (auto it = m_items.constBegin(); it != m_items.constEnd(); ++it) {
        QPoint p = it.value();
        QString uri = it.key();
        uris << (QPair<QString, QPoint> (uri, coordinateLocal2Global(p)));
    }

    return uris;
}

bool GScreen::uriIsOnScreen(const QString &uri) const
{
    return m_items.contains(uri);
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
    m_items.clear();
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
    return m_screen;
}

QPoint GScreen::placeItem(const QString &uri, QPoint lastPos, bool force)
{
    if (!isValidScreen() || "" == uri) {
        return INVALID_POS;
    }

    if (m_items.value(uri, INVALID_POS) != INVALID_POS) {
        m_items.remove(uri);
    }

    QPoint pos = INVALID_POS;
    int x = lastPos.x();
    int y = lastPos.y();
    while (x <= m_maxColumn && y <= m_maxRow) {
        // check if there is an index in this grid pos.
        auto tmp = QPoint(x, y);
        if (m_items.key(tmp).isEmpty()) {
            pos.setX(x);
            pos.setY(y);
            m_items.insert(uri, pos);
            return pos;
        } else {
            if (y + 1 <= m_maxRow) {
                y++;
            } else {
                y = 0;
                if (x + 1 <= m_maxColumn) {
                    x++;
                } else {
                    // out of grid, break
                    break;
                }
            }
            continue;
        }
    }

    // 强行放置到 (0, 0) 位置
    if (force) {
        m_items.insert(uri, QPoint(0, 0));
        return QPoint(0, 0);
    }

    return pos;
}

QPoint GScreen::itemGridPos(const QString &uri)
{
    if (m_items.contains(uri)) {
        QPoint rowColum = m_items[uri];
        return coordinateLocal2Global(rowColum);
    }

    return INVALID_POS;
}

void GScreen::makeItemGridPosInvalid(const QString &uri)
{
    m_items.remove(uri);

#if 0
    qDebug() << "-------------------------" << __FUNCTION__ << "-------------------------";
    for (auto u : m_items.keys()) {
        qDebug() << "uri:" << u;
    }
    qDebug() << "------------------------------------------------------------------------";
#endif
}

void GScreen::makeItemMetaPosInvalid(const QString &uri)
{
    m_itemsMetaPoses.remove(uri);
}

bool GScreen::isItemOutOfGrid(const QString &uri)
{
    auto pos = m_items.value(uri);

    if (pos == INVALID_POS) {
        return true;
    } else if (pos.x() > m_maxColumn || pos.y() > m_maxRow) {
        return false;
    }

    return true;
}

// 表示行列
QPoint GScreen::getItemRelatedPosition(const QString &uri)
{
    if (!m_screen || "" == uri) {
        return INVALID_POS;
    }

    QPoint pos = INVALID_POS;

    if (m_items.contains(uri))  pos = m_items[uri];
    if (INVALID_POS == pos)     pos = getMetaPos(uri);
    if (INVALID_POS != pos)     return pos;

    return INVALID_POS;
}

QPoint GScreen::getItemGlobalPosition(const QString &uri)
{
    if (!m_screen || "" == uri) {
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
    if (!m_screen) {
        return nullptr;
    }

    if (pos.x() <= m_maxColumn && pos.x() >= 0 && pos.y() <= m_maxRow && pos.y() >= 0) {
        return m_items.key(pos);
    } else {
        return nullptr;
    }
}

QPoint GScreen::coordinateGlobal2Local(const QPoint &pos) const
{
    QPoint posl = pos - m_screen->geometry().topLeft();

    return QPoint(posl.x() / m_gridSize.width(), posl.y() / m_gridSize.height());
}

QPoint GScreen::coordinateLocal2Global(const QPoint &pos) const
{
    QPoint posg(pos.x() * m_gridSize.width(), pos.y() * m_gridSize.height());

    return m_screen->geometry().topLeft() + posg;
}

bool GScreen::posAvailable(QPoint &p) const
{
    if (p.x() >= m_maxRow || p.y() >= m_maxColumn) {
        return false;
    }

    return true;
}

QString GScreen::getItemFromGlobalPosition(const QPoint &pos)
{
    if (!m_screen) {
        return nullptr;
    }

    return getItemFromRelatedPosition(coordinateGlobal2Local(pos));
}

// pos 表示行列
bool GScreen::setItemGridPos(const QString &uri, const QPoint &pos)
{
    auto currentGridPos = m_items.value(uri);
    if (currentGridPos == pos) {
        return true;
    }

    if (pos.x() > m_maxColumn || pos.y() > m_maxRow || pos.x() < 0 || pos.y() < 0 || iconIsConflict(pos)) {
        qWarning() << "invalid grid pos";
        return false;
    }

    auto itemOnTargetPos = m_items.key(pos);
    if (itemOnTargetPos.isEmpty()) {
        m_items.insert(uri, pos);
        return true;
    } else {
        return false;
    }
}

bool GScreen::setItemWithGlobalPos(const QString &uri, const QPoint &pos)
{
    if (m_screen && m_screen->availableGeometry().contains(pos)) {
        return setItemGridPos(uri, coordinateGlobal2Local(pos));
    }

    return false;
}

// pos 表示位置
bool GScreen::saveItemWithGlobalPos(const QString &uri, const QPoint &pos)
{
    if (m_screen && m_screen->geometry().contains(pos)) {
        return saveMetaPos(uri, coordinateGlobal2Local(pos));
    }

    return false;
}

void GScreen::rebindScreen(QScreen *screen)
{
    if (!screen) {
        qCritical()<<"invalid screen";
        return;
    }

    if (m_screen) {
        m_screen->disconnect(m_screen, &QScreen::geometryChanged, this, 0);
        m_screen->disconnect(m_screen, &QScreen::destroyed, this, 0);
    }

    QHash<QString, QPoint>  item = m_items;
    QHash<QString, QPoint>  itemPoss = m_itemsMetaPoses;

    // FIXME://
    m_geometry.adjust(m_panelMargins.left(), m_panelMargins.top(), -m_panelMargins.right(), -m_panelMargins.bottom());
    connect(m_screen, &QScreen::geometryChanged, this, &GScreen::onScreenGeometryChanged);
    connect(m_screen, &QScreen::destroyed, this, [=](){
        m_screen = nullptr;
        Q_EMIT screenVisibleChanged(false);
    });

    Q_EMIT screenVisibleChanged(true);
}

}
