#ifndef DESKTOPINDEXWIDGET_H
#define DESKTOPINDEXWIDGET_H

#include <QWidget>
#include <QStyleOptionViewItem>
#include <QModelIndex>

class QTextEdit;

namespace graceful
{

class DesktopViewDelegate;

class DesktopIndexWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DesktopIndexWidget(DesktopViewDelegate *delegate, const QStyleOptionViewItem &option, const QModelIndex &index, QWidget *parent = nullptr);
    ~DesktopIndexWidget();

protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

    void updateItem();

private:
    QStyleOptionViewItem m_option;
    QModelIndex m_index;
    const DesktopViewDelegate* m_delegate;
    QFont m_current_font;

    QRect m_text_rect;

    bool b_elide_text = false;
    const int ELIDE_TEXT_LENGTH = 40;
};

}

#endif // DESKTOPINDEXWIDGET_H
