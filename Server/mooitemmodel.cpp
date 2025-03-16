#include "mooitemmodel.h"

#include "objectmanager.h"

MooItemModel::MooItemModel( QObject *pParent )
	: QAbstractItemModel( pParent )
{
	ObjectManager		*OM = ObjectManager::instance();

	rootItem = std::make_unique<MooItem>( OBJECT_NONE );

	setupModelData( rootItem.get() );

	connect( OM, &ObjectManager::objectAdded, this, &MooItemModel::objectAdded );

	connect( OM, &ObjectManager::objectDeleted, this, &MooItemModel::objectDeleted );

	connect( OM, &ObjectManager::objectParentChanged, this, &MooItemModel::objectParentUpdated );
}

QModelIndex MooItemModel::index( int row, int column, const QModelIndex &parent ) const
{
	if (!hasIndex(row, column, parent))
		return {};

	MooItem *parentItem = parent.isValid()
							   ? static_cast<MooItem*>(parent.internalPointer())
							   : rootItem.get();

	if (auto *childItem = parentItem->child(row))
		return createIndex(row, column, childItem);

	return {};
}

QModelIndex MooItemModel::parent( const QModelIndex &index ) const
{
	if (!index.isValid())
		return {};

	auto *childItem = static_cast<MooItem*>(index.internalPointer());
	MooItem *parentItem = childItem->parentItem();

	return parentItem != rootItem.get()
		? createIndex(parentItem->row(), 0, parentItem) : QModelIndex{};
}

int MooItemModel::rowCount( const QModelIndex &parent ) const
{
	if (parent.column() > 0)
		return 0;

	const MooItem *parentItem = parent.isValid()
									 ? static_cast<const MooItem*>(parent.internalPointer())
									 : rootItem.get();

	return parentItem->childCount();
}

int MooItemModel::columnCount( const QModelIndex &parent ) const
{
	if (parent.isValid())
		return static_cast<MooItem*>(parent.internalPointer())->columnCount();
	return rootItem->columnCount();
}

QVariant MooItemModel::data( const QModelIndex &index, int role ) const
{
	if( !index.isValid() || role != Qt::DisplayRole )
	{
		return( QVariant() );
	}

	// ObjectId		Id = index.internalId();

	// if( !index.column() )
	// {
	// 	return( Id );
	// }

	// return( ObjectManager::instance()->objectName( Id ) );

	const auto *item = static_cast<const MooItem*>(index.internalPointer());
	return item->data(index.column());
}

Qt::ItemFlags MooItemModel::flags(const QModelIndex &index) const
{
	return index.isValid()
	? QAbstractItemModel::flags(index) : Qt::ItemFlags(Qt::NoItemFlags);
}


QVariant MooItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		switch (section) {
			case 0:
				return QString("Id");
			case 1:
				return QString("Name");
		}
	}
	return QVariant();
}

void MooItemModel::setupModelData( MooItem *parent )
{
	ObjectManager		*OM = ObjectManager::instance();

	ObjectIdVector		OIDV = OM->children( parent->mId );

	for( ObjectId &OID : OIDV )
	{
		parent->appendChild( std::make_unique<MooItem>( OID, parent ) );

		setupModelData( parent->m_childItems.back().get() );
	}
}

void MooItemModel::objectAdded(ObjectId pId)
{
	ObjectManager		*OM = ObjectManager::instance();

	ObjectIdVector		 ObjHie = OM->objectHierarchy( pId );

	QModelIndex			 MI;

	MooItem				*P = rootItem.get();

	while( !ObjHie.empty() )
	{
		ObjectId		 NID = ObjHie.takeFirst();

		bool found = false;

		while( !found )
		{
			const int rc = rowCount( MI );

			for( int i = 0 ; i < rc ; i++ )
			{
				QModelIndex		CI = index( i, 0, MI );

				MooItem			*C = static_cast<MooItem *>( CI.internalPointer() );

				if( CI.isValid() && C->mId == NID )
				{
					MI = CI;
					P  = C;

					found = true;

					break;
				}
			}

			if( !found )
			{
				int rc = rowCount( MI );

				beginInsertRows( MI, rc, rc + 1 );

				P->appendChild( std::make_unique<MooItem>( NID, P ) );

				endInsertRows();
			}
		}
	}
}

void MooItemModel::objectDeleted( ObjectId pId )
{
	ObjectManager		*OM = ObjectManager::instance();

	ObjectIdVector		 ObjHie = OM->objectHierarchy( pId );

	QModelIndex			 MI;

	MooItem				*P = rootItem.get();

	while( ObjHie.size() > 1 )
	{
		ObjectId		 NID = ObjHie.takeFirst();

		bool found = false;

		while( !found )
		{
			const int rc = rowCount( MI );

			for( int i = 0 ; i < rc ; i++ )
			{
				QModelIndex		CI = index( i, 0, MI );

				MooItem			*C = static_cast<MooItem *>( CI.internalPointer() );

				if( CI.isValid() && C->mId == NID )
				{
					MI = CI;
					P  = C;

					found = true;

					break;
				}
			}
		}
	}

	int		OIdx = P->childObjectRow( pId );

	if( OIdx >= 0 )
	{
		beginRemoveRows( MI, OIdx, OIdx );

		P->m_childItems.erase( P->m_childItems.begin() + OIdx );

		endRemoveRows();
	}
}

void MooItemModel::objectParentUpdated( ObjectId pObjectId, ObjectId pOldParentId, ObjectId pNewParentId )
{
	ObjectManager		*OM = ObjectManager::instance();

	ObjectIdVector		 OldObjHie = OM->objectHierarchy( pOldParentId );
	ObjectIdVector		 NewObjHie = OM->objectHierarchy( pNewParentId );

	QModelIndex			 OMIP;

	MooItem				*OP = rootItem.get();

	while( !OldObjHie.empty() )
	{
		ObjectId		 NID = OldObjHie.takeFirst();

		bool found = false;

		while( !found )
		{
			const int rc = rowCount( OMIP );

			for( int i = 0 ; i < rc ; i++ )
			{
				QModelIndex		CI = index( i, 0, OMIP );

				MooItem			*C = static_cast<MooItem *>( CI.internalPointer() );

				if( CI.isValid() && C->mId == NID )
				{
					OMIP = CI;
					OP  = C;

					found = true;

					break;
				}
			}
		}
	}

	QModelIndex			 NMIP;

	MooItem				*NP = rootItem.get();

	while( !NewObjHie.empty() )
	{
		ObjectId		 NID = NewObjHie.takeFirst();

		bool found = false;

		while( !found )
		{
			const int rc = rowCount( NMIP );

			for( int i = 0 ; i < rc ; i++ )
			{
				QModelIndex		CI = index( i, 0, NMIP );

				MooItem			*C = static_cast<MooItem *>( CI.internalPointer() );

				if( CI.isValid() && C->mId == NID )
				{
					NMIP = CI;
					NP  = C;

					found = true;

					break;
				}
			}
		}
	}

	int		OIdx = OP->childObjectRow( pObjectId );

	beginMoveRows( OMIP, OIdx, OIdx, NMIP, NP->childCount() );

	std::unique_ptr<MooItem> MI = std::move( OP->m_childItems.at( OIdx ) );

	OP->m_childItems.erase( OP->m_childItems.begin() + OIdx );

	MI->m_parentItem = NP;

	NP->appendChild( std::move( MI ) );

	endMoveRows();
}

