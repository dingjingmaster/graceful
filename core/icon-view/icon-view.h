#ifndef ICONVIEW_H
#define ICONVIEW_H

#include <QMap>
#include <QTimer>
#include <QQueue>
#include <QAbstractItemView>

class QRubberBand;

namespace graceful
{
class GScreen;
class FileModel;

class IconView : public QAbstractItemView
{
    friend class GScreen;
    friend class FileModel;
    friend class IconViewDelegate;
    Q_OBJECT
public:
    enum ZoomLevel
    {
        Invalid,
        Small,      // icon: 24x24; grid: 64x64;   hover rect = 60x60;   font: system*0.8
        Normal,     // icon: 48x48; grid: 96x96;   hover rect = 90x90;   font: system
        Large,      // icon: 64x64; grid: 115x135; hover rect = 105x118; font: system*1.2
        Huge        // icon: 96x96; grid: 140x170; hover rect = 120x140; font: system*1.4
    };
    Q_ENUM(ZoomLevel)

    explicit IconView(QWidget *parent = nullptr);

    void setGridSize(QSize size);
    GScreen* getScreen(int screenId);

    void addScreen(GScreen* screen);
    void removeScreen(GScreen* screen);
    void swaGScreen(GScreen* screen1, GScreen* screen2);

    QStringList getSelections();
    QString getIndexUri(const QModelIndex &index) const;
    QModelIndex findIndexByUri(const QString &uri) const;
    QModelIndex indexAt(const QPoint &point) const override;
    QRect visualRect(const QModelIndex &index) const override;

    bool isRenaming() const;
    bool isItemOverlapped(const QString &uri);
    bool isIndexOverlapped(const QModelIndex &index);
    bool trySetIndexToPos(const QModelIndex &index, const QPoint &pos);

    void scrollTo(const QModelIndex& index, ScrollHint hint) override {}

    void zoomIn();
    void zoomOut();
    ZoomLevel zoomLevel() const;

    void setRenaming (bool);
    void setEditFlag (bool);
    const QRect getBoundingRect();

    QSize getGridSize() const;
    QSize getIconSize() const;
    QPoint getMarginLeftTop() const;

    void setShowHidden();
    void setDefaultZoomLevel(ZoomLevel level);

    void updateItemPosByUri(const QString &uri, const QPoint &pos);
    QPoint getFileMetaInfoPos(const QString &uri) const;

    void refresh();

public Q_SLOTS:
    void editUri(const QString&uri);

private:
    GScreen* getScreenByPos (const QPoint& pos);

Q_SIGNALS:
    void doubleClickFile(QString uri);
    void deleteFileFromView(QString uri);
    void screenResolutionChanged(GScreen* screen);

protected:
    void keyPressEvent(QKeyEvent* e) override;
    void paintEvent(QPaintEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

    void dropEvent(QDropEvent *event) override;
    void dragMoveEvent(QDragMoveEvent* e) override;
    void dragEnterEvent(QDragEnterEvent* e) override;
    void startDrag(Qt::DropActions supportedActions) override;

    QStyleOptionViewItem viewOptions() const override;

    int verticalOffset() const override {return 0;}
    int horizontalOffset() const override {return 0;}

    bool isIndexHidden(const QModelIndex &index) const override {return false;}

    QRegion visualRegionForSelection(const QItemSelection &selection) const override;
    void setSelection(const QModelIndex& index, QItemSelectionModel::SelectionFlags command);
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) override;
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override {return QModelIndex();}

protected Q_SLOTS:
    void rowsInserted(const QModelIndex &parent, int start, int end) override;
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) override;
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>()) override;

    void saveItemsPositions();
    void handleGridSizeChanged();
    void handleScreenChanged(GScreen *screen);

    void relayoutItems(const QStringList &uris);

    GScreen* getItemScreen(const QString &uri);

private:
    bool                                mIsEdit = false;
    bool                                mIsRenaming = false;
    bool                                mShowHidden = false;
    bool                                mRealDoEdit = false;
    bool                                mCtrlKeyPressed = false;
    bool                                mCtrlOrShiftPressed = false;

    bool                                mSelectFlag = false;
    bool                                mDragFlag = false;

    QModelIndex                         mHoverIndex;
    QModelIndex                         mEditingIndex;


    QModelIndex                         mLastIndex;
    QModelIndexList                     mDragIndexes;

    ZoomLevel                           mZoomLevel = Invalid;
    QSize                               mGridSize = QSize(100, 150);
    FileModel*                          mModel;
    QList <GScreen*>                    mScreens;
    GScreen*                            mPrimaryScreen;

    QStringList                         mItems;
    QStringList                         mFloatItems;
    QMap<QString, QPoint>               mItemsPosesCached;

    QQueue<QString>                     mTobeRendered;

    QPoint                              mPressPos;
    QPoint                              mDragStartPos;
    QRubberBand*                        mRubberBand = nullptr;

    QTimer                              mEditTriggerTimer;
};

}

#endif
