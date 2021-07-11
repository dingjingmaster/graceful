#include "desktopfile-model.h"
#include <QtCore/private/qobject_p.h>

#include <QSize>
#include <QIcon>
#include <QDebug>
#include <QMimeData>

#include <syslog.h>

graceful::DesktopFileModel::DesktopFileModel(QObject *parent) : QAbstractItemModel(parent)
{
    qDebug() << __FUNCTION__ << __LINE__;
}

graceful::DesktopFileModel::~DesktopFileModel()
{
    qDebug() << __FUNCTION__ << __LINE__;
}

void graceful::DesktopFileModel::setRootPath(QString rootPath)
{
    qDebug() << __FUNCTION__ << __LINE__;
    // file exists?
    if (rootPath.isEmpty()) {
        return;
    }

    if (mCurrentPath) {
        delete mCurrentPath;
        mCurrentPath = nullptr;
    }

    if (!mItems.isEmpty()) {
        removeAll();
    }

    mCurrentPath = new File(rootPath);

    // enumerat

    // insert files
    QStringList ls;
    ls << "aa" << "bb" << "cc" << "dd" << "ee" << "ff";

    insertFiles(0, ls);
}

void graceful::DesktopFileModel::fetchMore(const QModelIndex &parent)
{
    qDebug() << __FUNCTION__ << __LINE__;
    Q_UNUSED(parent)
}

bool graceful::DesktopFileModel::canFetchMore(const QModelIndex &parent) const
{
    qDebug() << __FUNCTION__ << __LINE__;
    if (mCurrentPath.isNull()) {
        return false;
    }

    return true;
    Q_UNUSED(parent)
}

Qt::ItemFlags graceful::DesktopFileModel::flags(const QModelIndex &index) const
{
    qDebug() << __FUNCTION__ << __LINE__;
//    Qt::ItemFlags flags;
//    if(index.isValid()) {
//        flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
//        if(index.column() == ColumnFileName) {
//            flags |= (Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable);
//        }
//    } else {
//        flags = Qt::ItemIsDropEnabled;
//    }
    return/* flags |*/ Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
}

//bool graceful::DesktopFileModel::hasChildren(const QModelIndex &parent) const
//{
//    return false;
//}

//bool graceful::DesktopFileModel::insertRows(int row, int count, const QModelIndex &parent)
//{
//    qDebug() << __FUNCTION__ << __LINE__;
//    if (row < 0 || count < 1 || row > rowCount(parent)) {
//        return false;
//    }

//    beginInsertRows(QModelIndex(), row, row + count - 1);

//    for (int r = 0; r < count; ++r) {
//        mItems.append(new DesktopFileModelItem(QString("test")));
//    }

//    endInsertRows();

//    return true;
//}

//bool graceful::DesktopFileModel::insertColumns(int column, int count, const QModelIndex &parent)
//{
//    qDebug() << __FUNCTION__ << __LINE__;
//    return false;
//    Q_UNUSED(count)
//    Q_UNUSED(column)
//    Q_UNUSED(parent)
//}

//QMap<int, QVariant> graceful::DesktopFileModel::itemData(const QModelIndex& index) const
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

QVariant graceful::DesktopFileModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() > mItems.size() || index.column() >= NumOfColumns) {
        return QVariant();
    }

//    if ((role == Qt::TextAlignmentRole) || (role == Qt::ForegroundRole)) {
//        return int(Qt::AlignHCenter | Qt::AlignVCenter);
//    }

    DesktopFileModelItem* item = reinterpret_cast<DesktopFileModelItem*>(index.internalPointer());

    qDebug() << "row: " << index.row() << " colume: " << index.column() << " role" << role << " is display role:" << (Qt::DisplayRole == role);

    switch(role) {
    case Qt::ToolTipRole:
        return QVariant("makeTooltip(item)");
    case Qt::DisplayRole:  {
        switch(index.column()) {
        case ColumnFileName:
            return (item->uri());
        case ColumnFileType:
            return (item->uri());
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
        if(index.column() == 0) {
            return QVariant(item->icon());
        }
        break;
    }
    case Qt::EditRole: {
        if(index.column() == 0) {
            return (item->uri());
        }
        break;
    }
    case FileInfoRole:
        return (item->uri());
    case FileIsDirRole:
        return (item->uri());
    case FileIsCutRole:
        return false;
    }

    return QVariant();
}

QModelIndex graceful::DesktopFileModel::index(int row, int column, const QModelIndex &parent) const
{
    qDebug() << __FUNCTION__ << __LINE__ << "  index";
    if(row < 0 || row >= mItems.size() || column < 0 || column >= NumOfColumns) {
        return QModelIndex();
    }
    qDebug() << __FUNCTION__ << __LINE__ << "  index";

    const DesktopFileModelItem* item = mItems.at(row);

    return createIndex(row, column, (void*)item);
}

QVariant graceful::DesktopFileModel::headerData(int section, Qt::Orientation orientation, int role) const
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

//QModelIndexList graceful::DesktopFileModel::match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const
//{
//    qDebug() << __FUNCTION__ << __LINE__;
//    return QAbstractItemModel::match(start, role, value, hits, flags);
//}

QStringList graceful::DesktopFileModel::mimeTypes() const
{
    qDebug() << __FUNCTION__ << __LINE__;
    QStringList types = QAbstractItemModel::mimeTypes();

    types << QStringLiteral("XdndDirectSave0");
    types << QStringLiteral("text/uri-list");

    return types;
}

QMimeData *graceful::DesktopFileModel::mimeData(const QModelIndexList &indexes) const
{
    qDebug() << __FUNCTION__ << __LINE__;
    QMimeData* data = QAbstractItemModel::mimeData(indexes);
    QByteArray urilist, libfmUrilist;
    urilist.reserve(4096);
    libfmUrilist.reserve(4096);

    for(const auto& index : indexes) {
        DesktopFileModelItem* item = reinterpret_cast<DesktopFileModelItem*>(index.internalPointer());
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

//bool graceful::DesktopFileModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild)
//{
//    qDebug() << __FUNCTION__ << __LINE__;
//    return false;
//}

//bool graceful::DesktopFileModel::moveColumns(const QModelIndex &sourceParent, int sourceColumn, int count, const QModelIndex &destinationParent, int destinationChild)
//{
//    qDebug() << __FUNCTION__ << __LINE__;
//    return false;
//}

QModelIndex graceful::DesktopFileModel::parent(const QModelIndex &index) const
{
    qDebug() << __FUNCTION__ << __LINE__;

    if (!index.isValid()) {
        return QModelIndex();
    }

    qDebug() << __FUNCTION__ << " --- " << index.row() << "  --  " << index.column();

    const DesktopFileModelItem* item = mItems.at(index.row());

    return createIndex(index.row(), index.column(), (void*)&item);
}

int graceful::DesktopFileModel::rowCount(const QModelIndex &parent) const
{
    qDebug() << __FUNCTION__ << __LINE__ << " -- " << mItems.size();
    if (parent.isValid()) {
        return 0;
    }
    qDebug() << __FUNCTION__ << __LINE__ << " -- " << mItems.size();

    return mItems.size();
}

int graceful::DesktopFileModel::columnCount(const QModelIndex &parent) const
{
    qDebug() << __FUNCTION__ << __LINE__;
    if (parent.isValid()) {
        return 0;
    }

    return NumOfColumns;
}

bool graceful::DesktopFileModel::removeRows(int row, int count, const QModelIndex& parent)
{
    qDebug() << __FUNCTION__ << __LINE__;
    return true;

    Q_UNUSED(row)
    Q_UNUSED(count)
    Q_UNUSED(parent)
}

bool graceful::DesktopFileModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    qDebug() << __FUNCTION__ << __LINE__;
    return false;
    Q_UNUSED(count)
    Q_UNUSED(parent)
    Q_UNUSED(column)
}

bool graceful::DesktopFileModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    qDebug() << __FUNCTION__ << __LINE__;
    return false;
    Q_UNUSED(index)
    Q_UNUSED(roles)
}

bool graceful::DesktopFileModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    qDebug() << __FUNCTION__ << __LINE__;
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

bool graceful::DesktopFileModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    qDebug() << __FUNCTION__ << __LINE__;
    return false;
}

//QSize graceful::DesktopFileModel::span(const QModelIndex &index) const
//{
//    qDebug() << __FUNCTION__ << __LINE__;
//    return QSize(1, 1);
//}

void graceful::DesktopFileModel::sort(int column, Qt::SortOrder order)
{
    qDebug() << __FUNCTION__ << __LINE__;
    QAbstractItemModel::sort(column, order);
}

QModelIndex graceful::DesktopFileModel::sibling(int row, int column, const QModelIndex &index) const
{
    qDebug() << __FUNCTION__ << __LINE__;
    if (row == index.row() && column < NumOfColumns) {
        return createIndex(row, column, index.internalPointer());
    } else {
        return QAbstractItemModel::sibling(row, column, index);
    }
}

Qt::DropActions graceful::DesktopFileModel::supportedDragActions() const
{
    qDebug() << __FUNCTION__ << __LINE__;
    return Qt::CopyAction;
}

Qt::DropActions graceful::DesktopFileModel::supportedDropActions() const
{
    qDebug() << __FUNCTION__ << __LINE__;
    return QAbstractItemModel::supportedDropActions() | Qt::MoveAction;
}

bool graceful::DesktopFileModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    qDebug() << __FUNCTION__ << __LINE__;
    return QAbstractItemModel::dropMimeData(data, action, row, column, parent);
}

bool graceful::DesktopFileModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    qDebug() << __FUNCTION__ << __LINE__;
    return QAbstractItemModel::canDropMimeData(data, action, row, column, parent);
}

void graceful::DesktopFileModel::removeAll()
{
    qDebug() << __FUNCTION__ << __LINE__;
    if (mItems.empty()) {
        return;
    }

    beginRemoveRows(QModelIndex(), 0, mItems.size() - 1);
    for (auto it : mItems) {
        delete it;
    }
    mItems.clear();
    endRemoveRows();
}

void graceful::DesktopFileModel::insertFiles(int row, const QStringList &files)
{
    qDebug() << __FUNCTION__ << __LINE__;
    int filesNum = files.size();

    beginInsertRows(QModelIndex(), row, row + filesNum - 1);
    for (QString f : files) {
        DesktopFileModelItem* item = new DesktopFileModelItem(f);
        mItems.append(item);
        qDebug() << "insert:" << f;
    }
    endInsertRows();
}


graceful::DesktopFileModelItem::DesktopFileModelItem(QString uri)
{
    mFile = new File(uri);
}

graceful::DesktopFileModelItem::DesktopFileModelItem(DesktopFileModelItem &other)
{
    mFile = new File(other.mFile->uri());
}

QString graceful::DesktopFileModelItem::name() const
{
    return mFile->fileName();
}

QString graceful::DesktopFileModelItem::uri() const
{
    return mFile->uri();
}

QString graceful::DesktopFileModelItem::path() const
{
    return mFile->path();
}

QIcon graceful::DesktopFileModelItem::icon() const
{
    return mFile->icon();
}
