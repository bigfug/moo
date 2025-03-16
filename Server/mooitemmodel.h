#ifndef MOOITEMMODEL_H
#define MOOITEMMODEL_H

#include <QAbstractItemModel>

#include "mooglobal.h"
#include "objectmanager.h"

#include <vector>

class MooItem
{
public:
	explicit MooItem( ObjectId pId = OBJECT_NONE, MooItem *pParent = Q_NULLPTR )
		: mId( pId ), m_parentItem(pParent)
	{
	}

	void appendChild( std::unique_ptr<MooItem> &&child )
	{
		m_childItems.push_back(std::move(child));
	}

	MooItem *child(int row)
	{
		return row >= 0 && row < childCount() ? m_childItems.at(row).get() : nullptr;
	}

	int childCount() const
	{
		return int(m_childItems.size());
	}

	int columnCount() const
	{
		return( 2 );
	}

	QVariant data(int column) const
	{
		if( column == 0 )
		{
			return( mId );
		}

		if( column == 1 )
		{
			return( ObjectManager::instance()->objectName( mId ) );
		}

		return( QVariant() );
	}

	int row() const
	{
		if (m_parentItem == nullptr)
			return 0;

		const auto it = std::find_if(m_parentItem->m_childItems.cbegin(), m_parentItem->m_childItems.cend(),
									 [this](const std::unique_ptr<MooItem> &treeItem) {
										 return treeItem.get() == this;
									 });

		if (it != m_parentItem->m_childItems.cend())
			return std::distance(m_parentItem->m_childItems.cbegin(), it);

		Q_ASSERT(false); // should not happen
		return -1;
	}

	MooItem *childObject( ObjectId pId )
	{
		const auto it = std::find_if(m_childItems.cbegin(), m_childItems.cend(),
									 [pId](const std::unique_ptr<MooItem> &treeItem) {
										 return treeItem->mId == pId;
									 });

		if (it != m_parentItem->m_childItems.cend())
			return it->get();

		return( Q_NULLPTR );
	}

	int childObjectRow( ObjectId pId )
	{
		for( int i = 0 ; i < m_childItems.size() ; i++ )
		{
			if( m_childItems.at( i )->mId == pId )
			{
				return( i );
			}
		}

		return( -1 );
	}

	MooItem *parentItem()
	{
		return m_parentItem;
	}

public:
	ObjectId								mId;
	std::vector<std::unique_ptr<MooItem>> m_childItems;
	MooItem *m_parentItem;
};

class MooItemModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	Q_DISABLE_COPY_MOVE(MooItemModel)

	MooItemModel( QObject *pParent = Q_NULLPTR );

	~MooItemModel() = default;

	// QAbstractItemModel interface
public:
	virtual QModelIndex index(int row, int column, const QModelIndex &parent) const Q_DECL_OVERRIDE;
	virtual QModelIndex parent(const QModelIndex &child) const Q_DECL_OVERRIDE;
	virtual int rowCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
	virtual int columnCount(const QModelIndex &parent) const Q_DECL_OVERRIDE;
	virtual QVariant data(const QModelIndex &index, int role) const Q_DECL_OVERRIDE;
	virtual Qt::ItemFlags flags(const QModelIndex &index) const Q_DECL_OVERRIDE;

	// QAbstractItemModel interface
public:
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;

protected:
	void setupModelData( MooItem *parent );

protected slots:
	void objectAdded( ObjectId pId );
	void objectDeleted( ObjectId pId );

	void objectParentUpdated( ObjectId pObjectId, ObjectId pOldParentId, ObjectId pNewParentId );

private:
	std::unique_ptr<MooItem> rootItem;
};

#endif // MOOITEMMODEL_H
