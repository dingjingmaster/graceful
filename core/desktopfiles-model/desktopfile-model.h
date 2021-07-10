#ifndef DESKTOPFILEMODEL_H
#define DESKTOPFILEMODEL_H

#include "globals.h"
#include <QAbstractItemModel>

class QAbstractItemModelPrivate;

namespace graceful
{
class GRACEFUL_API DesktopFileModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit DesktopFileModel(QObject* parent = nullptr);
    ~DesktopFileModel() override;

    virtual void fetchMore(const QModelIndex &parent) override;
    virtual bool canFetchMore(const QModelIndex &parent) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    virtual bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    virtual QMap<int, QVariant> itemData(const QModelIndex &index) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    virtual QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith|Qt::MatchWrap)) const override;
    virtual QStringList mimeTypes() const override;
    virtual QMimeData* mimeData(const QModelIndexList &indexes) const override;
    virtual bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;
    virtual bool moveColumns(const QModelIndex &sourceParent, int sourceColumn, int count, const QModelIndex &destinationParent, int destinationChild) override;
    virtual QModelIndex parent(const QModelIndex &index) const override;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    virtual bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    virtual bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    virtual QSize span(const QModelIndex &index) const override;
    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    virtual QModelIndex sibling(int row, int column, const QModelIndex &index) const override;

    virtual Qt::DropActions supportedDragActions() const override;
    virtual Qt::DropActions supportedDropActions() const override;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;

protected:
    DesktopFileModel(QAbstractItemModelPrivate& dd, QObject *parent);


Q_SIGNALS:

private:
    Q_DISABLE_COPY(DesktopFileModel)
};
}


#endif // DESKTOPFILEMODEL_H
