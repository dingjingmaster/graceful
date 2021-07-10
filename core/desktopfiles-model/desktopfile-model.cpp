#include "desktopfile-model.h"
#include <QtCore/private/qobject_p.h>

#include <QSize>

graceful::DesktopFileModel::DesktopFileModel(QObject *parent) : QAbstractItemModel(parent)
{

}

graceful::DesktopFileModel::~DesktopFileModel()
{

}

void graceful::DesktopFileModel::fetchMore(const QModelIndex &parent)
{

}

bool graceful::DesktopFileModel::canFetchMore(const QModelIndex &parent) const
{
    return true;
}

Qt::ItemFlags graceful::DesktopFileModel::flags(const QModelIndex &index) const
{
    return Qt::NoItemFlags;
}

bool graceful::DesktopFileModel::hasChildren(const QModelIndex &parent) const
{
    return false;
}

bool graceful::DesktopFileModel::insertRows(int row, int count, const QModelIndex &parent)
{
    return true;
}

bool graceful::DesktopFileModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    return true;
}

QMap<int, QVariant> graceful::DesktopFileModel::itemData(const QModelIndex &index) const
{
    return QMap<int, QVariant>();
}

QVariant graceful::DesktopFileModel::data(const QModelIndex &index, int role) const
{
    return QVariant();
}

QModelIndex graceful::DesktopFileModel::index(int row, int column, const QModelIndex &parent) const
{
    return QModelIndex();
}

QVariant graceful::DesktopFileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

QModelIndexList graceful::DesktopFileModel::match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const
{
    return QModelIndexList();
}

QStringList graceful::DesktopFileModel::mimeTypes() const
{
    return QStringList();
}

QMimeData *graceful::DesktopFileModel::mimeData(const QModelIndexList &indexes) const
{
    return nullptr;
}

bool graceful::DesktopFileModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
{
    return false;
}

bool graceful::DesktopFileModel::moveColumns(const QModelIndex &sourceParent, int sourceColumn, int count, const QModelIndex &destinationParent, int destinationChild)
{
    return false;
}

QModelIndex graceful::DesktopFileModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int graceful::DesktopFileModel::rowCount(const QModelIndex &parent) const
{
    return 1;
}

int graceful::DesktopFileModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

bool graceful::DesktopFileModel::removeRows(int row, int count, const QModelIndex &parent)
{
    return true;
}

bool graceful::DesktopFileModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    return true;
}

bool graceful::DesktopFileModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    return true;
}

bool graceful::DesktopFileModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return true;
}

bool graceful::DesktopFileModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    return true;
}

QSize graceful::DesktopFileModel::span(const QModelIndex &index) const
{
    return QSize();
}

void graceful::DesktopFileModel::sort(int column, Qt::SortOrder order)
{

}

QModelIndex graceful::DesktopFileModel::sibling(int row, int column, const QModelIndex &index) const
{
    return QModelIndex();
}

Qt::DropActions graceful::DesktopFileModel::supportedDragActions() const
{
    return Qt::CopyAction;
}

Qt::DropActions graceful::DesktopFileModel::supportedDropActions() const
{
    return Qt::CopyAction;
}

bool graceful::DesktopFileModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    return true;
}

bool graceful::DesktopFileModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    return true;
}

graceful::DesktopFileModel::DesktopFileModel(QAbstractItemModelPrivate& dd, QObject *parent) : QAbstractItemModel(dd, parent)
{

}

