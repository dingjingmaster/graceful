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

    QPoint getGridCenterPoint(QPoint& pos);                                                       // 获取拖拽点

    QList<QPair<QString, QPoint>> getItemsAndPosAll();                                      // 输出 <uri, pos>, pos 表示位置
    QList<QPair<QString, QPoint>> getItemsAndPosOutOfScreen();                              // 输出 <uri, pos>, pos 表示位置

    bool posIsOnScreen(const QPoint& pos);                                                  // ok 传入表示位置
    QPoint itemGridPos(const QString &uri);                                                 // ok 获取全局位置 m_items, 传出表示位置
    bool isItemOutOfGrid(const QString &uri);                                               // ok
    bool uriIsOnScreen(const QString& uri) const;

    QPoint putIconOnScreen(const QString uri, QPoint start=QPoint(), bool force=false);     // 输出位置，输入 start=位置
    QStringList putIconsOnScreen (const QStringList uris, bool force=false);                // 将图标放到屏幕上, force=true，没有位置会放到屏幕起始处

    void makeAllItemsGridPosInvalid();
    void makeItemGridPosInvalid(const QString& uri);                                        // ok
    void makeItemMetaPosInvalid(const QString& uri);

    bool setItemWithGlobalPos(const QString & uri, const QPoint& pos);                      // ok 不改变 meta,传入 pos 表示位置
    bool saveItemWithGlobalPos(const QString& uri, const QPoint& pos);                      // ok 传入 pos 表示位置

    QPoint getItemGlobalPosition(const QString &uri);                                       // ok 优先 m_items 其次 meta 获取, 返回 pos 表示位置
    QString getItemFromGlobalPosition(const QPoint &pos);                                   // ok 传入 pos 表示位置
    QPoint getItemMetaInfoGridPos(const QString &uri);                                      // 传出位置

    void refresh();                                                                         // 只是检查文件是否重叠、越界，把重叠、越界的文件重新摆列

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
    void clearItems();                                                                      // ok

    int maxRow() const;                                                                     // ok
    int maxColumn() const;                                                                  // ok

    bool iconIsConflict(QPoint point);                                                      // 输入 行列，检查是否冲突

    QPoint getMetaPos (const QString& uri);                                                 // 传出行列
    bool saveMetaPos (const QString& uri, const QPoint& pos);                               // 传入行列

    bool setItemGridPos(const QString &uri, const QPoint &pos);                             // pos 表示 行列

    QPoint getItemRelatedPosition(const QString &uri);                                      // 返回 pos 表示行列
    QString getItemFromRelatedPosition(const QPoint &pos);                                  // 传入行列，传出行列

    QPoint coordinateGlobal2Local(const QPoint& pos) const;                                 // 全局转局部坐标
    QPoint coordinateLocal2Global(const QPoint& pos) const;                                 // 局部转全局坐标

    bool posAvailable(QPoint& p) const;

    QPoint placeItem(const QString &uri, QPoint lastPos=QPoint(), bool force=false);        // ok 传入行列 传出行列

private:
    int                             mMaxRow = 0;
    int                             mMaxColumn = 0;
    QRect                           mGeometry;
    QSize                           mGridSize;
    QMargins                        mPanelMargins;

    QHash<QString, QPoint>          mItems;                                                 // 图标在当前屏幕上的位置，可能与 meta 不一致，可能会导致冲突（待验证）
    QHash<QString, QPoint>          mItemsMetaPoses;                                        // 在当前屏幕的位置，与 meta 属性保持一致

    QScreen*                        mScreen = nullptr;
};

}

#endif // SCREEN_H
