#ifndef DesktopView2_H
#define DesktopView2_H

#include "screen.h"

#include <QMap>
#include <QTimer>
#include <QQueue>
#include <QAbstractItemView>

class QRubberBand;

namespace graceful
{
class DesktopFileModel;

class DesktopView : public QAbstractItemView
{
    friend class GScreen;
    friend class DesktopItemModel;
    friend class DesktopViewDelegate;
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

    explicit DesktopView(QWidget *parent = nullptr);

    void setGridSize(QSize size);
    GScreen* getScreen(int screenId);

    void addScreen(GScreen* screen);
    void removeScreen(GScreen* screen);
    void swaGScreen(GScreen* screen1, GScreen* screen2);

    QString getIndexUri(const QModelIndex &index) const;
    QModelIndex findIndexByUri(const QString &uri) const;
    QModelIndex indexAt(const QPoint &point) const override;
    QRect visualRect(const QModelIndex &index) const override;

    bool isRenaming() const;
    bool isItemOverlapped(const QString &uri);
    bool isIndexOverlapped(const QModelIndex &index);
    bool trySetIndexToPos(const QModelIndex &index, const QPoint &pos);

    void scrollTo(const QModelIndex& index, ScrollHint hint) override {}

    void _saveItemsPoses();

    void zoomIn();                              // ok
    void zoomOut();                             // ok
    ZoomLevel zoomLevel() const;                // ok

    void initMenu();                            // ok
    void initShoutCut();                        // ok

    void setRenaming (bool);                    // ok
    void setEditFlag (bool);                    // ok
    const QRect getBoundingRect();              // ???
    void openFileByUri(QString uri);            // ok

    QSize getGridSize() const;                  // ok
    QSize getIconSize() const;                  // ok
    QPoint getMarginLeftTop() const;            // ok

    void setShowHidden();
    void setDefaultZoomLevel(ZoomLevel level);                                              // ok

    void updateItemPosByUri(const QString &uri, const QPoint &pos);                         // ok
    QPoint getFileMetaInfoPos(const QString &uri) const;                                    // ok

    void refresh();

private:
    GScreen* getScreenByPos (const QPoint& pos);

Q_SIGNALS:
    void updateBWList ();
    void deleteFileFromView (QString uri);
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
    bool                                m_is_edit = false;
    bool                                m_is_renaming = false;
    bool                                m_show_hidden = false;
    bool                                m_real_do_edit = false;
    bool                                m_ctrl_key_pressed = false;
    bool                                m_ctrl_or_shift_pressed = false;

    bool                                m_select_flag = false;
    bool                                m_drag_flag = false;

    QModelIndex                         m_hover_index;
    QModelIndex                         m_editing_index;


    QModelIndex                         m_last_index;
    QModelIndexList                     m_drag_indexes;

    ZoomLevel                           m_zoom_level = Invalid;
    QSize                               m_gridSize = QSize(100, 150);
    DesktopFileModel*                   m_model;
    QList <GScreen*>                    m_screens;
    GScreen*                            m_primaryScreen;

    QStringList                         m_items;                        // 所有桌面图标 uri
    QStringList                         m_floatItems;                   // 当有拖拽或者libpeony文件操作触发时，固定所有float元素并记录metaInfo
    QMap<QString, QPoint>               m_itemsPosesCached;             //

    QQueue<QString>                     m_tobeRendered;                 // 将要重新绘制的图标

    QPoint                              m_press_pos;
    QPoint                              m_dragStartPos;
    QRubberBand*                        m_rubberBand = nullptr;

    QTimer                              m_edit_trigger_timer;
};

}

#endif // DesktopView_H
