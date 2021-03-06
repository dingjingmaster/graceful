#include "icon-view.h"

#include "gscreen.h"
#include "file/file.h"
#include "icon-view-delegate.h"
#include "file-model/file-model.h"

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
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDrag>
#include <QWindow>


#define SCREEN_ID                   1000
#define RELATED_GRID_POSITION       1001

#define INVALID_POS                 QPoint(-1, -1)

static bool iconSizeLessThan (const QPair<QString, QPoint> &p1, const QPair<QString, QPoint> &p2);

namespace graceful
{

IconView::IconView(QWidget *parent) : QAbstractItemView(parent)
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

    mLastIndex = QModelIndex();

    mEditTriggerTimer.setInterval(3000);
    mEditTriggerTimer.setSingleShot(true);

    setItemDelegate(new IconViewDelegate(this));

    setIconSize(QSize(64, 64));

    setDefaultZoomLevel(zoomLevel());

    installEventFilter(this);

    for (auto qscreen : qApp->screens()) {
        auto screen = new GScreen(qscreen, getGridSize(), this);
        addScreen(screen);
        if (qApp->primaryScreen() == qscreen) {
            mPrimaryScreen = screen;
        }
    }

    mRubberBand = new QRubberBand(QRubberBand::Rectangle, this);

    connect(qApp, &QGuiApplication::screenAdded, this, [=] (QScreen* screen) {
        GScreen* s = new GScreen(screen, getGridSize(), this);
        addScreen(s);
        for (auto sc : mScreens) {
            if (qApp->primaryScreen() == sc->getScreen()) {
                mPrimaryScreen = sc;
            }
        }
    });

    connect(qApp, &QGuiApplication::screenRemoved, this, [=] (QScreen* screen) {
        GScreen* s = nullptr;
        QList<QPair<QString, QPoint>> uris;
        QStringList ls;
        for (auto sc : mScreens) {
            if (sc->getScreen() == screen) {
                uris << sc->getItemsAndPosAll();
                s = sc;
                break;
            }
        }

        std::stable_sort(uris.begin(), uris.end(), iconSizeLessThan);
        for (auto u : uris) {
            ls << u.first;
            mItemsPosesCached.remove(u.first);
        }

        if (s) {mScreens.removeOne(s);}
        s->deleteLater();

        if (mScreens.size() == 0) {
            mPrimaryScreen->putIconsOnScreen(ls, true);
        } else {
            bool putOK = false;
            for (auto s1 : mScreens) {
                if (ls.isEmpty()) {
                    putOK = true;
                    break;
                }
                ls = s1->putIconsOnScreen(ls);
            }

            mPrimaryScreen->putIconsOnScreen(ls, true);
        }
    });

    connect(qApp, &QApplication::paletteChanged, this, [=] () {
        viewport()->update();
    });

    connect(qApp, &QGuiApplication::primaryScreenChanged, this, [=] (QScreen* prims) {
        GScreen* oldPrimaryScreen = mPrimaryScreen;
        for (auto s : mScreens) {
            if (prims == s->getScreen()) {
                mPrimaryScreen = s;
                break;
            }
        }

        swaGScreen(mPrimaryScreen, oldPrimaryScreen);
    });

    connect(this, &IconView::screenResolutionChanged, this, [=] (GScreen* s) {
        if (!s || !s->isValidScreen()) {
            return ;
        }

        QStringList uris = s->getAllItemsOnScreen();
        for (auto uri : uris) {
            mItemsPosesCached.remove(uri);
        }
    });
}

GScreen *IconView::getScreen(int screenId)
{
    if (mScreens.count() > screenId) {
        return mScreens.at(screenId);
    } else {
        return nullptr;
    }
}

void IconView::addScreen(GScreen *screen)
{
    connect(screen, &GScreen::screenVisibleChanged, this, [=] (bool visible) {
        if (!visible) {
            removeScreen(screen);
        }
    });
    mScreens<<screen;
}

void IconView::swaGScreen(GScreen *screen1, GScreen *screen2)
{
    if (!screen1 || !screen2 || screen1 == screen2) {
        return;
    }

    QStringList uris;
    uris << screen1->getAllItemsOnScreen();
    uris << screen2->getAllItemsOnScreen();

#if 0
    qDebug() << "DJ- ????????????:";
    qDebug() << "DJ- GScreen: " << screen1 << "  -- screen:" << screen1->getScreen();
    qDebug() << "DJ- GScreen: " << screen2 << "  -- screen:" << screen2->getScreen();
#endif

    screen1->swapScreen(*screen2);

    int index1 = mScreens.indexOf(screen1);
    int index2 = mScreens.indexOf(screen2);
    mScreens.replace(index1, screen2);
    mScreens.replace(index2, screen1);

#if 0
    qDebug() << "DJ- ????????????:";
    qDebug() << "DJ- GScreen: " << screen1 << "  -- screen:" << screen1->getScreen();
    qDebug() << "DJ- GScreen: " << screen2 << "  -- screen:" << screen2->getScreen();
#endif

#if 0
    qDebug() << "DJ- ????????????:";
    for (auto s : mScreens) {
        qDebug() << "DJ- " << s;
    }
#endif

    for (auto uri : uris) {
        mItemsPosesCached.remove(uri);
    }

    this->handleScreenChanged(screen1);
    this->handleScreenChanged(screen2);
}

QStringList IconView::getSelections()
{
    QStringList uris;

    auto indexes = selectionModel()->selection().indexes();

    for (auto index : indexes) {
        uris << index.data(Qt::UserRole).toString();
    }

    uris.removeDuplicates();

    return uris;
}

void IconView::removeScreen(GScreen *screen)
{
    if (!screen) {
        return;
    }

    this->handleScreenChanged(screen);
}

void IconView::setGridSize(QSize size)
{
    mGridSize = size;

    for (auto screen : mScreens) {
        screen->onScreenGridSizeChanged(getGridSize());
    }

    for (auto screen : mScreens) {
        screen->makeAllItemsGridPosInvalid();
    }
    refresh();
}

QRect IconView::visualRect(const QModelIndex &index) const
{
    auto marg = getMarginLeftTop();
    auto rect = QRect(QPoint(0, 0), getIconSize());

    if (index.isValid()) {
        QString uri = index.data(FileModel::FileUriRole).toString();

        if (mItemsPosesCached.contains(uri)) {
            rect.translate(mItemsPosesCached[uri]);
        } else {
            QPoint pos = getFileMetaInfoPos(uri);
            if (INVALID_POS != pos) {
                rect = QRect(pos, getIconSize());
            }
        }
    }

    rect.moveTo(rect.x() + marg.x(), rect.y() + marg.y());

    return rect;
}

bool IconView::isRenaming() const
{
    return mIsRenaming;
}

QModelIndex IconView::indexAt(const QPoint &point) const
{
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

QModelIndex IconView::findIndexByUri(const QString &uri) const
{
    if (model()) {
        for (int row = 0; row < model()->rowCount(); row++) {
            auto index = model()->index(row, 0);
            auto indexUri = getIndexUri(index);
            if (indexUri == uri) {
                return index;
            }
        }
    }

    return QModelIndex();
}

QString IconView::getIndexUri(const QModelIndex &index) const
{
    return index.data(FileModel::FileUriRole).toString();
}

bool IconView::trySetIndexToPos(const QModelIndex &index, const QPoint &pos)
{
    QString uri = getIndexUri(index);
    for (auto screen : mScreens) {
        if (!screen->isValidScreen()) {
            continue;
        }
        if (screen->setItemWithGlobalPos(uri, pos)) {
            mItemsPosesCached.remove(uri);
            for (auto screen : mScreens) {
                screen->makeItemGridPosInvalid(uri);
            }
            return screen->setItemWithGlobalPos(uri, pos);
        } else {
        }
    }
    return false;
}

bool IconView::isIndexOverlapped(const QModelIndex &index)
{
    return isItemOverlapped(getIndexUri(index));
}

bool IconView::isItemOverlapped(const QString &uri)
{
    auto itemPos = mItemsPosesCached.value(uri);

    for (auto item : mItems) {
        if (mItemsPosesCached.value(item) == itemPos && item != uri) {
            return true;
        }
    }

    return false;
}

void IconView::zoomIn()
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

void IconView::zoomOut()
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

QPoint IconView::getMarginLeftTop() const
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

QSize IconView::getGridSize() const
{
    QPoint mg = getMarginLeftTop();

    return QSize(mGridSize.width() + mg.x(), mGridSize.height() + mg.y());
}

QSize IconView::getIconSize() const
{
    return mGridSize;
}

void IconView::setShowHidden()
{
}

GScreen *IconView::getScreenByPos(const QPoint &pos)
{
    for (auto s : mScreens) {
        if (s->posIsOnScreen(pos)) {
            return s;
        }
    }

    return nullptr;
}

void IconView::keyPressEvent(QKeyEvent *e)
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
        mCtrlOrShiftPressed = true;
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return: {
    }
        break;
    default:
        return QAbstractItemView::keyPressEvent(e);
    }
}

void IconView::setDefaultZoomLevel(IconView::ZoomLevel level)
{
    mZoomLevel = level;
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
        mZoomLevel = Normal;
        setIconSize(QSize(48, 48));
        setGridSize(QSize(96, 106));
        break;
    }
}

void IconView::updateItemPosByUri(const QString &uri, const QPoint &pos)
{

}

IconView::ZoomLevel IconView::zoomLevel() const
{
    return Normal;
}

const QRect IconView::getBoundingRect()
{
    QRegion itemsRegion(0, 0, 90, 90);

    return itemsRegion.boundingRect();
}

QPoint IconView::getFileMetaInfoPos(const QString &uri) const
{
    QPoint poss = INVALID_POS;
    for (auto screen : mScreens) {
        auto pos = screen->getItemGlobalPosition(uri);
        if (pos != INVALID_POS) {
            poss = pos;
        }
    }

    if (poss == INVALID_POS) {
        for (auto s : mScreens) {
            QPoint p = s->putIconOnScreen(uri);
            if (INVALID_POS != p) {
                poss = p;
                break;
            }
        }
    }

    return poss;
}

void IconView::refresh()
{
    mItemsPosesCached.clear();

    for (auto s : mScreens) {
        s->refresh();
    }

    saveItemsPositions();
}

void IconView::editUri(const QString &uri)
{
//    File f(uri);
}

void IconView::setRenaming(bool r)
{
    mIsRenaming = r;
}

void IconView::setEditFlag(bool e)
{
    mIsEdit = e;
}

void IconView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter p(viewport());

    for (auto item : mItems) {
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

        if (mHoverIndex == index) {
            opt.state |= QStyle::State_MouseOver;
        } else {
            opt.state &= ~QStyle::State_MouseOver;
        }

        itemDelegate()->paint(&p, opt, index);
//        qApp->style()->drawControl(QStyle::CE_ItemViewItem, &opt, &p, this);
    }
}

void IconView::dropEvent(QDropEvent* event)
{
    mRealDoEdit = false;
    mEditTriggerTimer.stop();
    if (event->keyboardModifiers() & Qt::ControlModifier) {
        mCtrlKeyPressed = true;
    } else {
        mCtrlKeyPressed = false;
    }

    auto index = indexAt(event->pos());
    auto action = mCtrlKeyPressed ? Qt::CopyAction : Qt::MoveAction;
    if (this == event->source() && !mCtrlKeyPressed) {
//        bool move = false;

        QPoint tmp = event->pos();

        GScreen* dragScreen = getScreenByPos(mDragStartPos);
        GScreen* droGScreen = getScreenByPos(tmp);

        QPoint dragPoint = mDragStartPos;
        QPoint dropPoint = tmp;

        if (dragScreen && dragScreen->isValidScreen()) {
            dragPoint = dragScreen->getGridCenterPoint(dragPoint);
        }

        if (droGScreen && droGScreen->isValidScreen()) {
            dropPoint = droGScreen->getGridCenterPoint(dropPoint);
        }

        if (index.isValid()) {
            QString uri = index.data(FileModel::FileUriRole).toString();
            graceful::File file(uri);
            if (!file.isDir() || event->mimeData()->urls().contains(uri)) {
                return;
            }
//            for (auto uuri : event->mimeData()->urls()) {
//                if ("trash:///" == uuri.toDisplayString() || "computer:///" == uuri.toDisplayString()) {
//                    return;
//                }
//            }
//            mModel->dropMimeData(event->mimeData(), action, -1, -1, indexAt(event->pos()));
        } else {
            QPoint offset = dropPoint - dragPoint;
            auto indexes = selectedIndexes();
            QStringList itemsNeedBeRelayouted;
            for (auto index : indexes) {
                QString uri = getIndexUri(index);
                mItemsPosesCached.remove(uri);
                for (auto screen : mScreens) {
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

    QAbstractItemView::dropEvent(event);

    saveItemsPositions();
    viewport()->update();
    model()->dropMimeData(event->mimeData(), action, -1, -1, index);
    mDragFlag = false;
}

void IconView::dragMoveEvent(QDragMoveEvent* event)
{
    mRealDoEdit = false;
    if (event->keyboardModifiers() & Qt::ControlModifier) {
        mCtrlKeyPressed = true;
    } else {
        mCtrlKeyPressed = false;
    }

    auto action = mCtrlKeyPressed ? Qt::CopyAction : Qt::MoveAction;
    auto index = indexAt(event->pos());
    if (index.isValid() && index != mLastIndex) {
        QHoverEvent he(QHoverEvent::HoverMove, event->posF(), event->posF());
        viewportEvent(&he);
    } else {
        QHoverEvent he(QHoverEvent::HoverLeave, event->posF(), event->posF());
        viewportEvent(&he);
    }

//    if (event->isAccepted()) {
//        return;
//    }

//    if (this == event->source()) {
//        event->accept();
//        return QAbstractItemView::dragMoveEvent(event);
//    }

    QPoint pos = event->pos();
    if (pos.x() <= getGridSize().width() / 2 || pos.y() <= getGridSize().height() / 2) {
        event->ignore();
    } else {
        event->setDropAction(action);
        event->accept();
    }
}

void IconView::dragEnterEvent(QDragEnterEvent* e)
{
    mRealDoEdit = false;

    auto action = mCtrlKeyPressed ? Qt::CopyAction : Qt::MoveAction;
    qDebug()<<"drag enter event" <<action;
    if (e->mimeData()->hasUrls()) {
        e->setDropAction(action);
        e->accept();
    }

    if (e->source() == this) {
//        mRealDoEdit = selectedIndexes();
    }

    QAbstractItemView::dragEnterEvent(e);
}

void IconView::startDrag(Qt::DropActions supportedActions)
{
    mDragFlag = true;
    QAbstractItemView::startDrag(supportedActions);
    return;
    auto indexes = selectedIndexes();
    if (indexes.count() > 0) {
        auto pos = mapFromGlobal(QCursor::pos());
        qreal scale = 1.0;
        QWidget *window = this->window();
        if (window) {
            QWindow* windowHandle = window->windowHandle();
            if (windowHandle) {
                scale = windowHandle->devicePixelRatio();
            }
        }

        auto drag = new QDrag(this);
        drag->setMimeData(model()->mimeData(indexes));

        QRegion rect;
        QHash<QModelIndex, QRect> indexRectHash;
        for (auto index : indexes) {
            rect += (visualRect(index));
            indexRectHash.insert(index, visualRect(index));
        }

        QRect realRect = rect.boundingRect();
        QPixmap pixmap(realRect.size() * scale);
        pixmap.fill(Qt::transparent);
        pixmap.setDevicePixelRatio(scale);
        QPainter painter(&pixmap);
        for (auto index : indexes) {
            painter.save();
            painter.translate(indexRectHash.value(index).topLeft() - rect.boundingRect().topLeft());
            itemDelegate()->paint(&painter, viewOptions(), index);
            painter.restore();
        }

        drag->setPixmap(pixmap);
        drag->setHotSpot(pos - rect.boundingRect().topLeft() - QPoint(viewportMargins().left(), viewportMargins().top()));
        drag->setDragCursor(QPixmap(), mCtrlKeyPressed? Qt::CopyAction: Qt::MoveAction);
        drag->exec(mCtrlKeyPressed ? Qt::CopyAction : Qt::MoveAction);
    } else {
        QAbstractItemView::startDrag(supportedActions);
    }
}

void IconView::mousePressEvent(QMouseEvent* e)
{
    // init
    mRealDoEdit = false;
    mHoverIndex = QModelIndex();

    mPressPos = e->pos();
    mDragStartPos = e->pos();
    QModelIndex index = indexAt(e->pos());

    if (e->modifiers() & Qt::ControlModifier) {
        mCtrlKeyPressed = true;
    } else {
        mCtrlKeyPressed = false;
    }

    if (e->button() & Qt::LeftButton) {
        if (!index.isValid()) {
            mSelectFlag = true;
        }
    } else if (e->button() & Qt::RightButton) {
        if (index.isValid() && !selectionModel()->isSelected(index)) {
            mLastIndex = index;
            setSelection(index, QItemSelectionModel::ClearAndSelect);
        }
    }

    if (!mCtrlOrShiftPressed) {
        if (!indexAt(e->pos()).isValid()) {
            clearSelection();
        } else {
            auto index = indexAt(e->pos());
            mLastIndex = index;
        }
    }

    if (e->button() != Qt::LeftButton) {
        return;
    }

    QAbstractItemView::mousePressEvent(e);
}

void IconView::mouseMoveEvent(QMouseEvent* event)
{
    QModelIndex index = indexAt(event->pos());
    mHoverIndex = (!mSelectFlag && index.isValid()) ? index : QModelIndex();

    if (!indexAt(mDragStartPos).isValid() && event->buttons() & Qt::LeftButton) {
        if (mRubberBand->size().width() > 100 && mRubberBand->height() > 100) {
            mRubberBand->setVisible(true);
        }
        setSelection(mRubberBand->geometry(), QItemSelectionModel::Select | QItemSelectionModel::Deselect);
    } else {
        mRubberBand->setVisible(false);
    }

    mRubberBand->setGeometry(QRect(mDragStartPos, event->pos()).normalized());

    QAbstractItemView::mouseMoveEvent(event);
}

void IconView::mouseReleaseEvent(QMouseEvent *event)
{
    mRubberBand->hide();

    mSelectFlag = false;

    QAbstractItemView::mouseReleaseEvent(event);
}

void IconView::mouseDoubleClickEvent(QMouseEvent* event)
{
    mRealDoEdit = false;
    if (Qt::LeftButton == event->button()) {
        QModelIndex index = indexAt(event->pos());
        if (index.isValid()) {
            Q_EMIT doubleClickFile(index.data(FileModel::FileUriRole).toString());
        }
    }
}

QStyleOptionViewItem IconView::viewOptions() const
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

void IconView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    if (mRubberBand->isVisible()) {
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

QRegion IconView::visualRegionForSelection(const QItemSelection &selection) const
{
    QRegion visualRegion;

    for (auto index : selection.indexes()) {
        visualRegion += visualRect(index);
    }

    return visualRegion;
}

void IconView::setSelection(const QModelIndex &index, QItemSelectionModel::SelectionFlags command)
{
    if (!index.isValid())
        return;

    selectionModel()->select(index, command);
}

void IconView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    auto string = topLeft.data().toString();
    Q_UNUSED(bottomRight)
    Q_UNUSED(roles)
    auto demageRect = visualRect(topLeft);
    viewport()->update(demageRect);
}

void IconView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QStringList needPut;
    for (int i = start; i <= end ; i++) {
        auto index = model()->index(i, 0);

        if (index.isValid()) {
            bool success = false;
            QString uri = getIndexUri(index);
            mItems.append(uri);

            mItemsPosesCached.remove(uri);
            QPoint pos = getFileMetaInfoPos(uri);
            if (INVALID_POS != pos) {
                success = true;
            } else {
                mFloatItems << uri;
                for (auto screen : mScreens) {
                    auto gridPos = screen->putIconOnScreen(uri, QPoint(0, 0));
                    if (gridPos.x() >= 0) {
                        success = true;
                        break;
                    }
                }
            }

            if (!success) {
                mPrimaryScreen->putIconOnScreen(uri, QPoint(), true);
            }
        }
    }

    viewport()->update();
}

void IconView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(end)
    auto indexAboutToBeRemoved = model()->index(start, 0);

    mItemsPosesCached.remove(getIndexUri(indexAboutToBeRemoved));
    mItems.removeOne(getIndexUri(indexAboutToBeRemoved));
    mFloatItems.removeOne(getIndexUri(indexAboutToBeRemoved));
    for (auto screen : mScreens) {
        screen->makeItemGridPosInvalid(getIndexUri(indexAboutToBeRemoved));
    }

    relayoutItems(mFloatItems);

    viewport()->update();
}

void IconView::saveItemsPositions()
{
    QStringList itemOnAllScreen;
    for (auto screen : mScreens) {
        itemOnAllScreen << screen->getItemsVisibleOnScreen();
    }

    for (auto item : itemOnAllScreen) {
        bool isOverlapped = isItemOverlapped(item);
        if (!isOverlapped) {
            mFloatItems.removeOne(item);

            for (auto s : mScreens) {
                s->makeItemGridPosInvalid(item);
                s->makeItemMetaPosInvalid(item);
            }

            auto screen = getItemScreen(item);
            if (screen) {
                QPoint gridPos = screen->itemGridPos(item);
                if (INVALID_POS != gridPos) {
                    mItemsPosesCached.remove(item);
                    screen->saveItemWithGlobalPos(item, gridPos);
                }
            }
        }
    }
}

void IconView::handleScreenChanged(GScreen *screen)
{
    QList<QPair<QString, QPoint>> items = screen->getItemsAndPosAll();

    std::sort(items.begin(), items.end(), iconSizeLessThan);

    QPoint p (0, 0);
    bool full = false;
    bool allFull = false;
    for (auto it : items) {
        bool success = false;
        QString uri = it.first;
        QPoint pos = it.second;
        mItemsPosesCached.remove(uri);
        if (screen->posIsOnScreen(pos)) {
            screen->saveItemWithGlobalPos(uri, pos);
            continue;
        }

        screen->makeItemGridPosInvalid(uri);
        screen->makeItemMetaPosInvalid(uri);

        if (!full) {
            p = screen->putIconOnScreen(uri, p);
            if (INVALID_POS == p) {
                p = QPoint(0, 0);
                full = true;
            } else {
                screen->saveItemWithGlobalPos(uri, p);
            }
        }

        if(!allFull) {
            for (auto s : mScreens) {
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

        if (!success) {
            mPrimaryScreen->putIconOnScreen(uri, QPoint(0, 0), true);
            screen->saveItemWithGlobalPos(uri, p);
        }
    }

    saveItemsPositions();
    viewport()->update();
}

void IconView::handleGridSizeChanged()
{
    for (auto screen : mScreens) {
        screen->onScreenGridSizeChanged(getGridSize());
        handleScreenChanged(screen);
    }

    viewport()->update();
}

void IconView::relayoutItems(const QStringList &uris)
{
    for (auto uri : uris) {
        mItemsPosesCached.remove(uri);
        for (auto screen : mScreens) {
            if (screen->isValidScreen()) {
                screen->makeItemGridPosInvalid(uri);
                screen->makeItemMetaPosInvalid(uri);
            }
        }
    }

    bool success = false;
    for (auto uri : uris) {
        for (auto screen : mScreens) {
            QPoint pos = screen->putIconOnScreen(uri);
            if (INVALID_POS != pos) {
                success = true;
                screen->saveItemWithGlobalPos(uri, pos);
                break;
            }
        }

        if (!success) {
            mPrimaryScreen->putIconOnScreen(uri, QPoint(0, 0), true);
        }
    }
}

GScreen *IconView::getItemScreen(const QString &uri)
{
    for (auto screen : mScreens) {
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
