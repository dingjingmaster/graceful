#include "desktop-view.h"

#include "desktop-view-delegate.h"

#include <QDebug>
#include <QAction>
#include <QPainter>
#include <gio/gio.h>
#include <QDropEvent>
#include <QMessageBox>
#include <QMessageBox>
#include <QPaintEvent>
#include <QApplication>
#include <QStandardPaths>


#define SCREEN_ID                   1000
#define RELATED_GRID_POSITION       1001

#define INVALID_POS                 QPoint(-1, -1)

static bool iconSizeLessThan (const QPair<QString, QPoint> &p1, const QPair<QString, QPoint> &p2);

namespace graceful
{

DesktopView::DesktopView(QWidget *parent) : QAbstractItemView(parent)
{
    setAttribute(Qt::WA_AlwaysStackOnTop);
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setDragEnabled(true);
    setMouseTracking(true);
    setDefaultDropAction(Qt::MoveAction);
    setDragDropMode(QAbstractItemView::DragDrop);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    viewport()->setAutoFillBackground(false);

    m_last_index = QModelIndex();

    m_edit_trigger_timer.setInterval(3000);
    m_edit_trigger_timer.setSingleShot(true);

    setItemDelegate(new DesktopViewDelegate(this));

    QIcon::setThemeName("ukui-icon-theme-default");

    setIconSize(QSize(64, 64));

    initShoutCut();
    setDefaultZoomLevel(zoomLevel());

    installEventFilter(this);

    for (auto qscreen : qApp->screens()) {
        auto screen = new GScreen(qscreen, getGridSize(), this);
        addScreen(screen);
        if (qApp->primaryScreen() == qscreen) {
            m_primaryScreen = screen;
        }
    }

    m_rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

    connect(qApp, &QGuiApplication::screenAdded, this, [=] (QScreen* screen) {
        addScreen(new GScreen(screen, getGridSize(), this));
        for (auto sc : m_screens) {
            if (qApp->primaryScreen() == sc->getScreen()) {
                m_primaryScreen = sc;
            }
        }
    });

    connect(qApp, &QGuiApplication::screenRemoved, this, [=] (QScreen* screen) {
        GScreen* s = nullptr;
        QList<QPair<QString, QPoint>> uris;
        QStringList ls;
        for (auto sc : m_screens) {
            if (sc->getScreen() == screen) {
                uris << sc->getItemsAndPosAll();
                s = sc;
                break;
            }
        }

        std::stable_sort(uris.begin(), uris.end(), iconSizeLessThan);
        for (auto u : uris) {
            ls << u.first;
            m_itemsPosesCached.remove(u.first);
        }

        if (s) m_screens.removeOne(s);
        s->deleteLater();

        if (m_screens.size() == 0) {
            m_primaryScreen->putIconsOnScreen(ls, true);
        } else {
            bool putOK = false;
            for (auto s1 : m_screens) {
                if (ls.isEmpty()) {
                    putOK = true;
                    break;
                }
                ls = s1->putIconsOnScreen(ls);
            }

            m_primaryScreen->putIconsOnScreen(ls, true);
        }
    });

    connect(qApp, &QApplication::paletteChanged, this, [=] () {
        viewport()->update();
    });

    connect(qApp, &QGuiApplication::primaryScreenChanged, this, [=] (QScreen* prims) {
        qDebug() << "DJ- &QGuiApplication::primaryScreenChanged" << prims;

        GScreen* oldPrimaryScreen = m_primaryScreen;
        for (auto s : m_screens) {
            if (prims == s->getScreen()) {
                m_primaryScreen = s;
                break;
            }
        }
        qDebug() << "DJ- " << __FUNCTION__ << "  -- 7";

        swaGScreen(m_primaryScreen, oldPrimaryScreen);
        qDebug() << "DJ- " << __FUNCTION__ << "  -- 8";
    });

//    connect(m_model, &DesktopFileModel::fileDeleted, this, [=] (const QString& uri) {
//        qDebug() << "DJ- delete uri: " << uri << " from view";
//        m_items.removeOne(uri);
//        m_floatItems.removeOne(uri);
//        m_itemsPosesCached.remove(uri);
//        for (auto s : m_screens) {
//            s->makeItemGridPosInvalid(uri);
//            s->makeItemMetaPosInvalid(uri);
//        }
//    });

    connect(this, &DesktopView::screenResolutionChanged, this, [=] (GScreen* s) {
        if (!s || !s->isValidScreen()) {
            return ;
        }

        QStringList uris = s->getAllItemsOnScreen();
        for (auto uri : uris) {
            m_itemsPosesCached.remove(uri);
        }
    });
}

GScreen *DesktopView::getScreen(int screenId)
{
    if (m_screens.count() > screenId) {
        return m_screens.at(screenId);
    } else {
        return nullptr;
    }
}

void DesktopView::addScreen(GScreen *screen)
{
    connect(screen, &GScreen::screenVisibleChanged, this, [=] (bool visible) {
        if (!visible) {
            removeScreen(screen);
        }
    });
    m_screens<<screen;
}

void DesktopView::swaGScreen(GScreen *screen1, GScreen *screen2)
{
    if (!screen1 || !screen2 || screen1 == screen2) {
        qCritical() << "DJ- invalide screen arguments " << screen1 << "  ---  " << screen2;
        return;
    }

    QStringList uris;
    uris << screen1->getAllItemsOnScreen();
    uris << screen2->getAllItemsOnScreen();

#if 0
    qDebug() << "DJ- 改变之前:";
    qDebug() << "DJ- GScreen: " << screen1 << "  -- screen:" << screen1->getScreen();
    qDebug() << "DJ- GScreen: " << screen2 << "  -- screen:" << screen2->getScreen();
#endif

    screen1->swapScreen(*screen2);

    // list操作
    int index1 = m_screens.indexOf(screen1);
    int index2 = m_screens.indexOf(screen2);
    m_screens.replace(index1, screen2);
    m_screens.replace(index2, screen1);

#if 0
    qDebug() << "DJ- 改变之前:";
    qDebug() << "DJ- GScreen: " << screen1 << "  -- screen:" << screen1->getScreen();
    qDebug() << "DJ- GScreen: " << screen2 << "  -- screen:" << screen2->getScreen();
#endif

#if 0
    qDebug() << "DJ- 最后次序:";
    for (auto s : m_screens) {
        qDebug() << "DJ- " << s;
    }
#endif

    for (auto uri : uris) {
        m_itemsPosesCached.remove(uri);
    }

    this->handleScreenChanged(screen1);
    this->handleScreenChanged(screen2);
}

void DesktopView::removeScreen(GScreen *screen)
{
    if (!screen) {
        qCritical()<<"invalid screen id";
        return;
    }

    this->handleScreenChanged(screen);
}

void DesktopView::setGridSize(QSize size)
{
    m_gridSize = size;

    for (auto screen : m_screens) {
        screen->onScreenGridSizeChanged(getGridSize());
    }

    for (auto screen : m_screens) {
        screen->makeAllItemsGridPosInvalid();
    }
    refresh();
}

QRect DesktopView::visualRect(const QModelIndex &index) const
{
    auto marg = getMarginLeftTop();
    auto rect = QRect(QPoint(0, 0), getIconSize());

    if (index.isValid()) {
        QString uri = index.data(Qt::UserRole).toString();

        if (m_itemsPosesCached.contains(uri)) {
            rect.translate(m_itemsPosesCached[uri]);
            qDebug() << "m_itemsPosesCached" << uri << m_itemsPosesCached[uri];
        } else {
            QPoint pos = getFileMetaInfoPos(uri);
            qDebug() << "getFileMetaInfoPos:" << uri << pos;
            if (INVALID_POS != pos) {
                rect = QRect(pos, getIconSize());
            }
        }
    }

    rect.moveTo(rect.x() + marg.x(), rect.y() + marg.y());

    return rect;
}

bool DesktopView::isRenaming() const
{
    return m_is_renaming;
}

QModelIndex DesktopView::indexAt(const QPoint &point) const
{
    // FIXME://优化
    QList<QRect> visualRects;
    for (int row = 0; row < model()->rowCount(); row++) {
        auto index = model()->index(row, 0);
        visualRects<<visualRect(index);
    }

    for (auto rect : visualRects) {
        if (rect.contains(point)) {
            int row = visualRects.indexOf(rect);
            return model()->index(row, 0);
        }
    }
    return QModelIndex();
}

QModelIndex DesktopView::findIndexByUri(const QString &uri) const
{
    return QModelIndex();
}

QString DesktopView::getIndexUri(const QModelIndex &index) const
{
    return index.data(Qt::UserRole).toString();
}

bool DesktopView::trySetIndexToPos(const QModelIndex &index, const QPoint &pos)
{
    QString uri = getIndexUri(index);
    for (auto screen : m_screens) {
        if (!screen->isValidScreen()) {
            continue;
        }
        if (screen->setItemWithGlobalPos(uri, pos)) {
            m_itemsPosesCached.remove(uri);
            //清空其它屏幕关于此index的gridPos?
            for (auto screen : m_screens) {
                screen->makeItemGridPosInvalid(uri);
            }
            //效率？
            return screen->setItemWithGlobalPos(uri, pos);
        } else {
            //不改变位置
        }
    }
    return false;
}

bool DesktopView::isIndexOverlapped(const QModelIndex &index)
{
    return isItemOverlapped(getIndexUri(index));
}

bool DesktopView::isItemOverlapped(const QString &uri)
{
    auto itemPos = m_itemsPosesCached.value(uri);

    for (auto item : m_items) {
        if (m_itemsPosesCached.value(item) == itemPos && item != uri) {
            return true;
        }
    }

    return false;
}

void DesktopView::_saveItemsPoses()
{
    this->saveItemsPositions();
}

void DesktopView::zoomIn()
{
    switch (zoomLevel()) {
    case Small:
        setDefaultZoomLevel(Normal);
        break;
    case Normal:
        setDefaultZoomLevel(Large);
        break;
    case Large:
        setDefaultZoomLevel(Huge);
        break;
    default:
        break;
    }
}

void DesktopView::zoomOut()
{
    switch (zoomLevel()) {
    case Huge:
        setDefaultZoomLevel(Large);
        break;
    case Large:
        setDefaultZoomLevel(Normal);
        break;
    case Normal:
        setDefaultZoomLevel(Small);
        break;
    default:
        break;
    }
}

void DesktopView::initMenu()
{

}

void DesktopView::initShoutCut()
{
    QAction *copyAction = new QAction(this);
    copyAction->setShortcut(QKeySequence::Copy);
    addAction(copyAction);

    QAction *cutAction = new QAction(this);
    cutAction->setShortcut(QKeySequence::Cut);
    addAction(cutAction);

    QAction *pasteAction = new QAction(this);
    pasteAction->setShortcut(QKeySequence::Paste);
    addAction(pasteAction);

    //add CTRL+D for delete operation
    auto trashAction = new QAction(this);
    trashAction->setShortcuts(QList<QKeySequence>()<<Qt::Key_Delete<<QKeySequence(Qt::CTRL + Qt::Key_D));
    addAction(trashAction);

    QAction *undoAction = new QAction(this);
    undoAction->setShortcut(QKeySequence::Undo);
    addAction(undoAction);

    QAction *redoAction = new QAction(this);
    redoAction->setShortcut(QKeySequence::Redo);
    addAction(redoAction);

    QAction *zoomInAction = new QAction(this);
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    connect(zoomInAction, &QAction::triggered, [=]() {
        this->zoomIn();
    });
    addAction(zoomInAction);

    QAction *zoomOutAction = new QAction(this);
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOutAction, &QAction::triggered, [=]() {
        this->zoomOut();
    });
    addAction(zoomOutAction);

    QAction *renameAction = new QAction(this);
    renameAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_E));
    addAction(renameAction);

    QAction *removeAction = new QAction(this);
    removeAction->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Delete));
    addAction(removeAction);

    auto propertiesWindowAction = new QAction(this);
    propertiesWindowAction->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::ALT + Qt::Key_Return) << QKeySequence(Qt::ALT + Qt::Key_Enter));
    addAction(propertiesWindowAction);

    auto newFolderAction = new QAction(this);
    newFolderAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_N));
    addAction(newFolderAction);

    QAction *refreshWinAction = new QAction(this);
    refreshWinAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_R));
    connect(refreshWinAction, &QAction::triggered, [=]() {
        this->refresh();
    });
    addAction(refreshWinAction);

    QAction *reverseSelectAction = new QAction(this);
    reverseSelectAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L));
    addAction(reverseSelectAction);

    QAction *normalIconAction = new QAction(this);
    normalIconAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_0));
    connect(normalIconAction, &QAction::triggered, [=]() {
        if (this->zoomLevel() == DesktopView::Normal)
            return;
        this->setDefaultZoomLevel(DesktopView::Normal);
    });
    addAction(normalIconAction);

    auto refreshAction = new QAction(this);
    refreshAction->setShortcut(Qt::Key_F5);
    connect(refreshAction, &QAction::triggered, this, [=]() {
        this->refresh();
    });
    addAction(refreshAction);

    QAction *editAction = new QAction(this);
    editAction->setShortcuts(QList<QKeySequence>() << QKeySequence(Qt::ALT + Qt::Key_E) << Qt::Key_F2);
    addAction(editAction);

    //show hidden action
    QAction *showHiddenAction = new QAction(this);
    showHiddenAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_H));
    addAction(showHiddenAction);
    connect(showHiddenAction, &QAction::triggered, this, [=]() {
        setShowHidden();
    });

    auto cancelAction = new QAction(this);
    cancelAction->setShortcut(Qt::Key_Escape);
    addAction(cancelAction);
}

QPoint DesktopView::getMarginLeftTop() const
{
    QPoint p(10, 5);
    float level = 1;

    switch (zoomLevel()) {
    case Small:
        level = 0.8;
        break;
    case Large:
        level = 1.2;
        break;
    case Huge:
        level = 1.4;
        break;
    default:
        break;
    }

    return p * level;
}

void DesktopView::openFileByUri(QString uri)
{

}

QSize DesktopView::getGridSize() const
{
    QPoint mg = getMarginLeftTop();

    return QSize(m_gridSize.width() + mg.x(), m_gridSize.height() + mg.y());
}

QSize DesktopView::getIconSize() const
{
    return m_gridSize;
}

void DesktopView::setShowHidden()
{
}

GScreen *DesktopView::getScreenByPos(const QPoint &pos)
{
    for (auto s : m_screens) {
        if (s->posIsOnScreen(pos)) {
            return s;
        }
    }

    return nullptr;
}

void DesktopView::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Home: {
        auto boundingRect = getBoundingRect();
        QRect homeRect = QRect(boundingRect.topLeft(), getGridSize());
        while (!indexAt(homeRect.center()).isValid()) {
            homeRect.translate(0, getGridSize().height());
        }
        auto homeIndex = indexAt(homeRect.center());
        selectionModel()->select(homeIndex, QItemSelectionModel::SelectCurrent);
        break;
    }
    case Qt::Key_End: {
        auto boundingRect = getBoundingRect();
        QRect endRect = QRect(boundingRect.bottomRight(), getGridSize());
        endRect.translate(-getGridSize().width(), -getGridSize().height());
        while (!indexAt(endRect.center()).isValid()) {
            endRect.translate(0, -getGridSize().height());
        }
        auto endIndex = indexAt(endRect.center());
        selectionModel()->select(endIndex, QItemSelectionModel::SelectCurrent);
        break;
    }
    case Qt::Key_Up: {
        return;
    }
    case Qt::Key_Down: {
        return;
    }
    case Qt::Key_Left:
    case Qt::Key_Right:
        return;
    case Qt::Key_Shift:
    case Qt::Key_Control:
        m_ctrl_or_shift_pressed = true;
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return: {
    }
        break;
    default:
        return QAbstractItemView::keyPressEvent(e);
    }
}

void DesktopView::setDefaultZoomLevel(DesktopView::ZoomLevel level)
{
    m_zoom_level = level;
    switch (level) {
    case Small:
        setIconSize(QSize(24, 24));
        setGridSize(QSize(64, 74));
        break;
    case Large:
        setIconSize(QSize(64, 64));
        setGridSize(QSize(115, 145));
        break;
    case Huge:
        setIconSize(QSize(96, 96));
        setGridSize(QSize(140, 180));
        break;
    default:
        m_zoom_level = Normal;
        setIconSize(QSize(48, 48));
        setGridSize(QSize(96, 106));
        break;
    }
}

void DesktopView::updateItemPosByUri(const QString &uri, const QPoint &pos)
{

}

DesktopView::ZoomLevel DesktopView::zoomLevel() const
{
    return Normal;
}

const QRect DesktopView::getBoundingRect()
{
    QRegion itemsRegion(0, 0, 90, 90);

    return itemsRegion.boundingRect();
}

QPoint DesktopView::getFileMetaInfoPos(const QString &uri) const
{
    QPoint poss = INVALID_POS;
    for (auto screen : m_screens) {
        auto pos = screen->getItemGlobalPosition(uri);
        if (pos != INVALID_POS) {
            poss = pos;
        }
    }

    // 针对第一次没有 meta 处理
    if (poss == INVALID_POS) {
        for (auto s : m_screens) {
            QPoint p = s->putIconOnScreen(uri);
            if (INVALID_POS != p) {
                poss = p;
                break;
            }
        }
    }

    return poss;
}

void DesktopView::refresh()
{
    m_itemsPosesCached.clear();

    for (auto s : m_screens) {
        s->refresh();
    }

    saveItemsPositions();
}

void DesktopView::setRenaming(bool r)
{
    m_is_renaming = r;
}

void DesktopView::setEditFlag(bool e)
{
    m_is_edit = e;
}

void DesktopView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter p(viewport());

    for (auto item : m_items) {
        auto index = findIndexByUri(item);

        QStyleOptionViewItem opt = viewOptions();

        opt.rect = visualRect(index);

        opt.text = index.data().toString();
        opt.icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));

        opt.state |= QStyle::State_Enabled;

        if (selectedIndexes().contains(index)) {
            opt.state |= QStyle::State_Selected;
        } else {
            opt.state &= ~QStyle::State_Selected;
        }

        if (m_hover_index == index) {
            opt.state |= QStyle::State_MouseOver;
        } else {
            opt.state &= ~QStyle::State_MouseOver;
        }

        qDebug() << "paint uri:" << item << "  -- pos:" << opt.rect;

        itemDelegate()->paint(&p, opt, index);
    }
}

void DesktopView::dropEvent(QDropEvent* event)
{
    m_real_do_edit = false;

    m_edit_trigger_timer.stop();

    if (event->keyboardModifiers() & Qt::ControlModifier) {
        m_ctrl_key_pressed = true;
    } else {
        m_ctrl_key_pressed = false;
    }

    auto action = m_ctrl_key_pressed ? Qt::CopyAction : Qt::MoveAction;
    if (this == event->source()) {
        QPoint tmp = event->pos();

        GScreen* dragScreen = getScreenByPos(m_dragStartPos);
        GScreen* droGScreen = getScreenByPos(tmp);

        QPoint dragPoint = m_dragStartPos;
        QPoint dropPoint = tmp;

        if (dragScreen && dragScreen->isValidScreen()) {
            dragPoint = dragScreen->getGridCenterPoint(dragPoint);
        }

        if (droGScreen && droGScreen->isValidScreen()) {
            dropPoint = droGScreen->getGridCenterPoint(dropPoint);
        }

        auto index = indexAt(dropPoint);
        if (index.isValid()) {
        } else {
            // change icon's position
            QPoint offset = dropPoint - dragPoint;
            auto indexes = selectedIndexes();
            QStringList itemsNeedBeRelayouted;
            for (auto index : indexes) {
                QString uri = getIndexUri(index);
                m_itemsPosesCached.remove(uri);
                for (auto screen : m_screens) {
                    screen->makeItemGridPosInvalid(uri);
                    screen->makeItemMetaPosInvalid(uri);
                }
            }

            for (auto index : indexes) {
                bool successed = false;
                QString uri = getIndexUri(index);
                auto sourceRect = visualRect(index);
                GScreen* srcScreen = getScreenByPos(sourceRect.topLeft());
                sourceRect.translate(offset);
                GScreen* destScreen = getScreenByPos(sourceRect.topLeft());
                if (srcScreen && destScreen) {
                    if (srcScreen->isValidScreen() && srcScreen == destScreen) {
                        successed = srcScreen->setItemWithGlobalPos(uri, sourceRect.topLeft());
                        srcScreen->saveItemWithGlobalPos(uri, sourceRect.topLeft());
                    } else if (destScreen->isValidScreen()) {
                        successed = destScreen->setItemWithGlobalPos(uri, sourceRect.topLeft());
                        destScreen->saveItemWithGlobalPos(uri, sourceRect.topLeft());
                    }
                }

                if (!successed) {
                    itemsNeedBeRelayouted << uri;
                }
            }

            relayoutItems(itemsNeedBeRelayouted);
        }
    }

    saveItemsPositions();
    viewport()->update();
    m_drag_flag = false;
}

void DesktopView::dragMoveEvent(QDragMoveEvent* event)
{
    m_real_do_edit = false;
    if (event->keyboardModifiers() & Qt::ControlModifier) {
        m_ctrl_key_pressed = true;
    } else {
        m_ctrl_key_pressed = false;
    }


    QPoint pos = event->pos();
    if (pos.x() <= getGridSize().width() / 2 || pos.y() <= getGridSize().height() / 2) {
        event->ignore();
    } else {
        event->accept();
    }
}

void DesktopView::dragEnterEvent(QDragEnterEvent *e)
{

}

void DesktopView::startDrag(Qt::DropActions supportedActions)
{
    m_drag_flag = true;
    QAbstractItemView::startDrag(supportedActions);
}

void DesktopView::mousePressEvent(QMouseEvent* e)
{
    // init
    m_real_do_edit = false;
    m_hover_index = QModelIndex();

    m_press_pos = e->pos();
    m_dragStartPos = e->pos();
    QModelIndex index = indexAt(e->pos());

    if (e->button() & Qt::LeftButton) {
        if (!index.isValid()) {
            m_select_flag = true;
        }
    } else if (e->button() & Qt::RightButton) {
        if (index.isValid() && !selectionModel()->isSelected(index)) {
            m_last_index = index;
            setSelection(index, QItemSelectionModel::ClearAndSelect);
        }
    }

    if (e->modifiers() & Qt::ControlModifier) {
        m_ctrl_key_pressed = true;
    } else {
        m_ctrl_key_pressed = false;
    }

    if (!m_ctrl_or_shift_pressed) {
        if (!indexAt(e->pos()).isValid()) {
            clearSelection();
        } else {
            auto index = indexAt(e->pos());
            m_last_index = index;
        }
    }

    if (e->button() != Qt::LeftButton) {
        return;
    }

    QAbstractItemView::mousePressEvent(e);

    qDebug() << m_dragStartPos;
}

void DesktopView::mouseMoveEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    m_hover_index = (!m_select_flag && index.isValid()) ? index : QModelIndex();

    if (!indexAt(m_dragStartPos).isValid() && event->buttons() & Qt::LeftButton) {
        if (m_rubberBand->size().width() > 100 && m_rubberBand->height() > 100) {
            m_rubberBand->setVisible(true);
        }
        setSelection(m_rubberBand->geometry(), QItemSelectionModel::Select | QItemSelectionModel::Deselect);
    } else {
        m_rubberBand->setVisible(false);
    }

    m_rubberBand->setGeometry(QRect(m_dragStartPos, event->pos()).normalized());

    QAbstractItemView::mouseMoveEvent(event);
}

void DesktopView::mouseReleaseEvent(QMouseEvent *event)
{
    m_rubberBand->hide();

    m_select_flag = false;

    QAbstractItemView::mouseReleaseEvent(event);
}

void DesktopView::mouseDoubleClickEvent(QMouseEvent* event)
{
    QPoint tmp = event->pos();
    QPoint p = getScreenByPos(tmp)->getGridCenterPoint(tmp);
    QModelIndex index = indexAt(p);

    if (!m_is_edit) {
        if (event->button() & Qt::LeftButton && index.isValid()) {
            openFileByUri(index.data(Qt::UserRole).toString());
        }
    }

    m_real_do_edit = false;
}

QStyleOptionViewItem DesktopView::viewOptions() const
{
    QStyleOptionViewItem item;
    item.palette.setBrush(QPalette::Text, Qt::white);
    item.decorationAlignment = Qt::AlignHCenter|Qt::AlignBottom;
    item.decorationSize = iconSize();
    item.decorationPosition = QStyleOptionViewItem::Position::Top;
    item.displayAlignment = Qt::AlignHCenter|Qt::AlignTop;
    item.features = QStyleOptionViewItem::HasDecoration|QStyleOptionViewItem::HasDisplay|QStyleOptionViewItem::WrapText;
    item.font = qApp->font();
    item.fontMetrics = qApp->fontMetrics();
    return item;
}

void DesktopView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    if (m_rubberBand->isVisible()) {
        for (int row = 0; row < model()->rowCount(); row++) {
            auto index = model()->index(row, 0);
            auto indexRect = visualRect(index);
            if (rect.intersects(indexRect)) {
                selectionModel()->select(index, QItemSelectionModel::Select);
            } else {
                selectionModel()->select(index, QItemSelectionModel::Deselect);
            }
        }
    } else {
        auto index = indexAt(rect.topLeft());
        selectionModel()->select(index, command);
    }
}

QRegion DesktopView::visualRegionForSelection(const QItemSelection &selection) const
{
    QRegion visualRegion;

    for (auto index : selection.indexes()) {
        visualRegion += visualRect(index);
    }

    return visualRegion;
}

void DesktopView::setSelection(const QModelIndex &index, QItemSelectionModel::SelectionFlags command)
{
    if (!index.isValid())
        return;

    selectionModel()->select(index, command);
}

void DesktopView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    auto string = topLeft.data().toString();
    Q_UNUSED(bottomRight)
    Q_UNUSED(roles)
    auto demageRect = visualRect(topLeft);
    viewport()->update(demageRect);
}

void DesktopView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QStringList needPut;
    for (int i = start; i <= end ; i++) {
        auto index = model()->index(i, 0);

        if (index.isValid()) {
            bool success = false;
            QString uri = getIndexUri(index);
            m_items.append(uri);

            // 优先获取 meta 信息
            m_itemsPosesCached.remove(uri);
            QPoint pos = getFileMetaInfoPos(uri);
            if (INVALID_POS != pos) {
                success = true;
            } else {
                m_floatItems << uri;
                for (auto screen : m_screens) {
                    auto gridPos = screen->putIconOnScreen(uri, QPoint(0, 0));
                    if (gridPos.x() >= 0) {
                        success = true;
                        break;
                    }
                }
            }

            if (!success) {
                m_primaryScreen->putIconOnScreen(uri, QPoint(), true);
            }
        }
    }

    viewport()->update();
}

void DesktopView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(end)
    auto indexAboutToBeRemoved = model()->index(start, 0);

    m_itemsPosesCached.remove(getIndexUri(indexAboutToBeRemoved));
    m_items.removeOne(getIndexUri(indexAboutToBeRemoved));
    m_floatItems.removeOne(getIndexUri(indexAboutToBeRemoved));
    for (auto screen : m_screens) {
        screen->makeItemGridPosInvalid(getIndexUri(indexAboutToBeRemoved));
    }

    // 重排浮动元素
    relayoutItems(m_floatItems);

    viewport()->update();
}

void DesktopView::saveItemsPositions()
{
    //非越界元素的确认，越界元素不应该保存位置
    QStringList itemOnAllScreen;
    for (auto screen : m_screens) {
        itemOnAllScreen << screen->getItemsVisibleOnScreen();
    }

    for (auto item : itemOnAllScreen) {
        //检查当前位置是否有重叠，如果有，则不确认
        bool isOverlapped = isItemOverlapped(item);
        if (!isOverlapped) {
            //从浮动元素中排除
            m_floatItems.removeOne(item);

            for (auto s : m_screens) {
                s->makeItemGridPosInvalid(item);
                s->makeItemMetaPosInvalid(item);
            }

            //设置metainfo
            auto screen = getItemScreen(item);
            if (screen) {
                QPoint gridPos = screen->itemGridPos(item);
                if (INVALID_POS != gridPos) {
                    m_itemsPosesCached.remove(item);
                    screen->saveItemWithGlobalPos(item, gridPos);
                }
            }
        }
    }
}

void DesktopView::handleScreenChanged(GScreen *screen)
{
    QList<QPair<QString, QPoint>> items = screen->getItemsAndPosAll();

    std::sort(items.begin(), items.end(), iconSizeLessThan);

    QPoint p (0, 0);
    bool full = false;              // 当前屏幕是否已满
    bool allFull = false;           // 所有屏幕已满
    for (auto it : items) {
        bool success = false;
        QString uri = it.first;
        QPoint pos = it.second;
        m_itemsPosesCached.remove(uri);
        if (screen->posIsOnScreen(pos)) {
            screen->saveItemWithGlobalPos(uri, pos);
            continue;
        }

        screen->makeItemGridPosInvalid(uri);
        screen->makeItemMetaPosInvalid(uri);

        // 放置失败或者不在屏幕上，先尝试本屏放置
        if (!full) {
            p = screen->putIconOnScreen(uri, p);
            if (INVALID_POS == p) {
                p = QPoint(0, 0);
                full = true;
            } else {
                screen->saveItemWithGlobalPos(uri, p);
            }
        }

        // 放置到其它屏幕
        if(!allFull) {
            for (auto s : m_screens) {
                if (s != screen) {
                    p = s->putIconOnScreen(uri, p);
                    if (INVALID_POS != p) {
                        success = true;
                        screen->saveItemWithGlobalPos(uri, p);
                        continue;
                    }
                }
                p = QPoint(0, 0);
            }
        }

        // 所有屏幕没有合适位置放主屏
        if (!success) {
            m_primaryScreen->putIconOnScreen(uri, QPoint(0, 0), true);
            screen->saveItemWithGlobalPos(uri, p);
        }
    }

    saveItemsPositions();
    viewport()->update();
}

void DesktopView::handleGridSizeChanged()
{
    // 保持index相对的grid位置不变，对越界图标进行处理
    for (auto screen : m_screens) {
        screen->onScreenGridSizeChanged(getGridSize());
        // 更新屏幕内图标的位置

        // 记录越界图标
        handleScreenChanged(screen);
    }
    // 重排越界图标

    viewport()->update();
}

void DesktopView::relayoutItems(const QStringList &uris)
{
    for (auto uri : uris) {
        m_itemsPosesCached.remove(uri);
        for (auto screen : m_screens) {
            if (screen->isValidScreen()) {
                screen->makeItemGridPosInvalid(uri);
                screen->makeItemMetaPosInvalid(uri);
            }
        }
    }

    bool success = false;
    for (auto uri : uris) {
        for (auto screen : m_screens) {
            QPoint pos = screen->putIconOnScreen(uri);
            if (INVALID_POS != pos) {
                success = true;
                screen->saveItemWithGlobalPos(uri, pos);
                break;
            }
        }

        if (!success) {
            // 可以优化，明确主屏已满情况下，可以直接放到 QPoint(0, 0); 另加 api
            m_primaryScreen->putIconOnScreen(uri, QPoint(0, 0), true);
        }
    }
}

GScreen *DesktopView::getItemScreen(const QString &uri)
{
    for (auto screen : m_screens) {
        if (screen->isValidScreen()) {
            if (screen->uriIsOnScreen(uri)) {
                return screen;
            }
        }
    }

    return nullptr;
}
}

static bool iconSizeLessThan (const QPair<QString, QPoint> &p1, const QPair<QString, QPoint> &p2)
{
    if (p1.second.x() > p2.second.x())
        return false;

    if ((p1.second.x() == p2.second.x()))
        return p1.second.y() < p2.second.y();

    return true;
}
