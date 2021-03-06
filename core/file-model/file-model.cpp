#include "file-model.h"

#include <QSize>
#include <QIcon>
#include <QIcon>
#include <QDebug>
#include <QMimeData>
#include <QtCore/private/qobject_p.h>

#include "log/log.h"
#include "file/file.h"
#include "file/file-enumerator.h"

graceful::FileModel::FileModel(QObject *parent) : QAbstractItemModel(parent)
{

}

graceful::FileModel::~FileModel()
{

}

void graceful::FileModel::setRootPath(QString rootPath)
{
    // file exists?
    gf_return_if_fail(!rootPath.isEmpty());

    if (mCurrentPath) {
        delete mCurrentPath;
        mCurrentPath = nullptr;
    }
    mCurrentPath = new File(rootPath);

    if (!mItems.isEmpty()) {
        removeAll();
    }

    // enumerate
    auto fileEnum = new FileEnumerator(this);
    fileEnum->setEnumerateDirectory(rootPath);
    fileEnum->setAutoDelete();
    fileEnum->enumerateAsync();
    fileEnum->connect(fileEnum, &FileEnumerator::enumerateFinished, this, [=] (bool res) {
        if (res) {
            insertFiles(0, fileEnum->getChildrenUris());
        } else {
            log_debug("enumerator error!");
        }
    });
}

void graceful::FileModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent)
}

bool graceful::FileModel::canFetchMore(const QModelIndex &parent) const
{
    gf_return_val_if_fail(mCurrentPath && mCurrentPath->isValid(), false);

    return true;
    Q_UNUSED(parent)
}

Qt::ItemFlags graceful::FileModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags;
    if(index.isValid()) {
        flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        if(index.column() == ColumnFileName) {
            flags |= (Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable);
        }
    } else {
        flags = Qt::ItemIsDropEnabled;
    }
    return flags;
}

//bool graceful::FileModel::hasChildren(const QModelIndex &parent) const
//{
//    return false;
//}

//bool graceful::FileModel::insertRows(int row, int count, const QModelIndex &parent)
//{
//    qDebug() << __FUNCTION__ << __LINE__;
//    if (row < 0 || count < 1 || row > rowCount(parent)) {
//        return false;
//    }

//    beginInsertRows(QModelIndex(), row, row + count - 1);

//    for (int r = 0; r < count; ++r) {
//        mItems.append(new FileModelItem(QString("test")));
//    }

//    endInsertRows();

//    return true;
//}

//bool graceful::FileModel::insertColumns(int column, int count, const QModelIndex &parent)
//{
//    qDebug() << __FUNCTION__ << __LINE__;
//    return false;
//    Q_UNUSED(count)
//    Q_UNUSED(column)
//    Q_UNUSED(parent)
//}

//QMap<int, QVariant> graceful::FileModel::itemData(const QModelIndex& index) const
//{
//    qDebug() << __FUNCTION__ << __LINE__;
//    if (!checkIndex(index, CheckIndexOption::IndexIsValid | CheckIndexOption::ParentIsInvalid)) {
//        return QMap<int, QVariant>{};
//    }

//    const QVariant displayData = mItems.at(index.row())->uri();
//    return QMap<int, QVariant>{{
//        std::make_pair<int>(Qt::DisplayRole, displayData),
//        std::make_pair<int>(Qt::EditRole, displayData)
//    }};

//    return QMap<int, QVariant>();
//}

QVariant graceful::FileModel::data(const QModelIndex &index, int role) const
{
    gf_return_val_if_fail(index.isValid() && index.row() <= mItems.size() && index.column() < NumOfColumns, QVariant());

    FileModelItem* item = reinterpret_cast<FileModelItem*>(index.internalPointer());

    switch(role) {
    case Qt::ToolTipRole:
        return QVariant("makeTooltip(item)");
    case Qt::DisplayRole:  {
        switch(index.column()) {
        case ColumnFileName:
            return item->name();
        case ColumnFileType:
            return item->name();
        case ColumnFileMTime:
            return (item->uri());
        case ColumnFileCrTime:
            return (item->uri());
        case ColumnFileDTime:
            return (item->uri());
        case ColumnFileSize:
            return (item->uri());
        case ColumnFileOwner:
            return (item->uri());
        case ColumnFileGroup:
            return (item->uri());
        }
        break;
    }
    case Qt::DecorationRole: {
        return item->icon();
    }
    case Qt::EditRole: {
        if(index.column() == 0) {
            return (item->name());
        }
        break;
    }
    case FileUriRole:
    case FileInfoRole:
        return (item->uri());
    case FileIsDirRole:
        return (item->uri());
    case FileIsCutRole:
        return false;
    }

    return QVariant();
}

QModelIndex graceful::FileModel::index(int row, int column, const QModelIndex &parent) const
{
    gf_return_val_if_fail(row >= 0 && row < mItems.size() && column >= 0 && column < NumOfColumns, QModelIndex());

    const FileModelItem* item = mItems.at(row);

    return createIndex(row, column, (void*)item);
}

QVariant graceful::FileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(role == Qt::DisplayRole) {
        if(orientation == Qt::Horizontal) {
            QString title;
            switch(section) {
            case ColumnFileName:
                title = tr("Name");
                break;
            case ColumnFileType:
                title = tr("Type");
                break;
            case ColumnFileSize:
                title = tr("Size");
                break;
            case ColumnFileMTime:
                title = tr("Modified");
                break;
            case ColumnFileCrTime:
                title = tr("Created");
                break;
            case ColumnFileDTime:
                title = tr("Deleted");
                break;
            case ColumnFileOwner:
                title = tr("Owner");
                break;
            case ColumnFileGroup:
                title = tr("Group");
                break;
            }
            return QVariant(title);
        }
    }
    return QVariant();
}

QStringList graceful::FileModel::mimeTypes() const
{
    QStringList types = QAbstractItemModel::mimeTypes();

    types << QStringLiteral("XdndDirectSave0");
    types << QStringLiteral("text/uri-list");

    return types;
}

QMimeData *graceful::FileModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData* data = QAbstractItemModel::mimeData(indexes);
    QByteArray urilist, libfmUrilist;
    urilist.reserve(4096);
    libfmUrilist.reserve(4096);

    for(const auto& index : indexes) {
        FileModelItem* item = reinterpret_cast<FileModelItem*>(index.internalPointer());
//        if(item && item->mInfo) {
//            auto path = item->mInfo->path();
//            if(path.isValid()) {
//                // get the list that will be used by internal DND
//                auto uri = path.uri();
//                libfmUrilist.append(uri.get());
//                libfmUrilist.append('\n');

//                if(auto localPath = path.localPath()) {
//                    QUrl url = QUrl::fromLocalFile(QString::fromUtf8(localPath.get()));
//                    urilist.append(url.toEncoded());
//                } else {
//                    urilist.append(uri.get());
//                }
//                urilist.append('\n');
//            }
//        }
    }
    data->setData(QStringLiteral("text/uri-list"), urilist);

    return QAbstractItemModel::mimeData(indexes);
}

QModelIndex graceful::FileModel::parent(const QModelIndex &index) const
{
    gf_return_val_if_fail(index.isValid(), QModelIndex());

    const FileModelItem* item = mItems.at(index.row());

    return createIndex(index.row(), index.column(), (void*)&item);
}

int graceful::FileModel::rowCount(const QModelIndex &parent) const
{
    gf_return_val_if_fail(!parent.isValid(), 0);

    return mItems.size();
}

int graceful::FileModel::columnCount(const QModelIndex &parent) const
{
    gf_return_val_if_fail(!parent.isValid(), 0);

    return NumOfColumns;
}

bool graceful::FileModel::removeRows(int row, int count, const QModelIndex& parent)
{
    return true;

    Q_UNUSED(row)
    Q_UNUSED(count)
    Q_UNUSED(parent)
}

bool graceful::FileModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    return false;
    Q_UNUSED(count)
    Q_UNUSED(parent)
    Q_UNUSED(column)
}

bool graceful::FileModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    return false;
    Q_UNUSED(index)
    Q_UNUSED(roles)
}

bool graceful::FileModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() >= 0 && index.row() < mItems.size() && (role == Qt::EditRole || role == Qt::DisplayRole)) {
//        const QString valueString = value.toString();
//        if (mItems.at(index.row()) == valueString) {
//            return true;
//        }
//        lst.replace(index.row(), valueString);
//        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
//        return true;
    }
    return false;
}

bool graceful::FileModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    return false;
}

void graceful::FileModel::sort(int column, Qt::SortOrder order)
{
    QAbstractItemModel::sort(column, order);
}

QModelIndex graceful::FileModel::sibling(int row, int column, const QModelIndex &index) const
{
    if (row == index.row() && column < NumOfColumns) {
        return createIndex(row, column, index.internalPointer());
    } else {
        return QAbstractItemModel::sibling(row, column, index);
    }
}

Qt::DropActions graceful::FileModel::supportedDragActions() const
{
    return Qt::CopyAction;
}

Qt::DropActions graceful::FileModel::supportedDropActions() const
{
    return QAbstractItemModel::supportedDropActions() | Qt::MoveAction;
}

bool graceful::FileModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    return QAbstractItemModel::dropMimeData(data, action, row, column, parent);
}

bool graceful::FileModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    return QAbstractItemModel::canDropMimeData(data, action, row, column, parent);
}

void graceful::FileModel::removeAll()
{
    gf_return_if_fail(!mItems.empty());

    beginRemoveRows(QModelIndex(), 0, mItems.size() - 1);
    for (auto it : mItems) {
        delete it;
    }
    mItems.clear();
    endRemoveRows();
}

void graceful::FileModel::insertFiles(int row, const QStringList &files)
{
    int filesNum = files.size();

    gf_return_if_fail(filesNum > 0);

    beginInsertRows(QModelIndex(), row, row + filesNum - 1);
    for (QString f : files) {
        log_debug("insert file:%s", f.toUtf8().constData());
        FileModelItem* item = new FileModelItem(f);
        mItems.append(item);
    }
    endInsertRows();
}


graceful::FileModelItem::FileModelItem(QString uri)
{
    mFile = new File(uri);
}

graceful::FileModelItem::FileModelItem(FileModelItem &other)
{
    mFile = new File(other.mFile->uri());
}

QString graceful::FileModelItem::name() const
{
    return mFile->fileName();
}

QString graceful::FileModelItem::uri() const
{
    return mFile->uri();
}

QString graceful::FileModelItem::path() const
{
    return mFile->path();
}

QIcon graceful::FileModelItem::icon() const
{
    return mFile->icon();
}
