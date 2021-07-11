#ifndef DesktopViewDELEGATE_H
#define DesktopViewDELEGATE_H

#include <QStyledItemDelegate>

class QPushButton;

namespace graceful
{

class DesktopView;

class DesktopViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    explicit DesktopViewDelegate(QObject *parent = nullptr);
    ~DesktopViewDelegate() override;

    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    DesktopView *getView() const;

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QPushButton *m_styled_button;
};
}

#endif // DesktopViewDELEGATE_H
