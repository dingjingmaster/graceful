#ifndef FILEMODEL_H
#define FILEMODEL_H

#include <QList>
#include <QString>
#include <QAbstractItemModel>

#include "globals.h"

class QAbstractItemModelPrivate;

namespace graceful
{

class File;
class GRACEFUL_API FileModelItem
{
public:
    explicit FileModelItem (QString uri);
    FileModelItem(FileModelItem& other);

    QString name() const;
    QString uri() const;
    QString path() const;

    QIcon icon() const;

private:
    File*                   mFile;                   // use graceful::File
};


class GRACEFUL_API FileModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Role
    {
        FileInfoRole = Qt::UserRole,
        FileIsDirRole,
        FileIsCutRole,
        FileUriRole
    };

    enum ColumnId
    {
        ColumnFileName,
        ColumnFileType,
        ColumnFileSize,
        ColumnFileMTime,
        ColumnFileCrTime,
        ColumnFileDTime,
        ColumnFileOwner,
        ColumnFileGroup,
        NumOfColumns
    };

    explicit FileModel(QObject* parent = nullptr);
    ~FileModel() override;

    // specific api
    void setRootPath(QString rootPath);


    // override
    /**
     * @brief list current folder's file
     */
    virtual void fetchMore(const QModelIndex &parent) override;
    virtual bool canFetchMore(const QModelIndex &parent) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const override;

    /**
     * @brief file has parent is folder
     */
//    virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;
//    virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
//    virtual bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

//    virtual QMap<int, QVariant> itemData(const QModelIndex &index) const override;

    /**
     * @brief
     */
    virtual QVariant data(const QModelIndex &index, int role) const override;

    /**
     * @brief
     */
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

//    virtual QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits = 1, Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchStartsWith|Qt::MatchWrap)) const override;
    virtual QStringList mimeTypes() const override;
    virtual QMimeData* mimeData(const QModelIndexList &indexes) const override;
//    virtual bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;
//    virtual bool moveColumns(const QModelIndex &sourceParent, int sourceColumn, int count, const QModelIndex &destinationParent, int destinationChild) override;
    virtual QModelIndex parent(const QModelIndex &index) const override;

    /**
     * @brief
     * data row counts MUST!
     */
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * @brief
     * data column counts  MUST!
     */
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    virtual bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    virtual bool setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles) override;

    /**
     * @brief modify file
     */
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

//    virtual QSize span(const QModelIndex &index) const override;
    virtual void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
    virtual QModelIndex sibling(int row, int column, const QModelIndex &index) const override;

    virtual Qt::DropActions supportedDragActions() const override;
    virtual Qt::DropActions supportedDropActions() const override;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    virtual bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;


private:
    /**
     * @brief
     * clear all items in model
     */
    void removeAll();

    void insertFiles(int row, const QStringList& files);

Q_SIGNALS:

private:
    File*                                           mCurrentPath = nullptr;
    QList<FileModelItem*>                           mItems;

    Q_DISABLE_COPY(FileModel)
};
}


#endif // FileModel_H
