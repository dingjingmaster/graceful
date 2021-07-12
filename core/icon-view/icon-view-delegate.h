#ifndef ICONVIEWDELEGATE_H
#define ICONVIEWDELEGATE_H

#include <QStyledItemDelegate>

class QPushButton;

namespace graceful
{

class IconView;

class IconViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    explicit IconViewDelegate(QObject *parent = nullptr);
    ~IconViewDelegate() override;

    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    IconView* getView() const;

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QPushButton*        mStyledButton;
};
}

#endif // IconViewDelegate_H
