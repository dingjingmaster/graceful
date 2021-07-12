#ifndef ICONINDEXWIDGET_H
#define ICONINDEXWIDGET_H

#include <QWidget>
#include <QStyleOptionViewItem>
#include <QModelIndex>

class QTextEdit;

namespace graceful
{

class IconViewDelegate;

class IconIndexWidget : public QWidget
{
    Q_OBJECT
public:
    explicit IconIndexWidget(IconViewDelegate *delegate, const QStyleOptionViewItem &option, const QModelIndex &index, QWidget *parent = nullptr);
    ~IconIndexWidget();

protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

    void updateItem();

private:
    QStyleOptionViewItem            mOption;
    QModelIndex                     mIndex;
    const IconViewDelegate*         mDelegate;

    QFont                           mCurrentFont;
    QRect                           mTextRect;

    bool                            mElideText = false;
    const int                       ELIDE_TEXT_LENGTH = 40;
};

}

#endif // DESKTOPINDEXWIDGET_H
