#include "mooitemmodel.h"

#include "objectmanager.h"

MooItemModel::MooItemModel( QObject *pParent )
	: QAbstractItemModel( pParent )
{

}


QModelIndex MooItemModel::index( int row, int column, const QModelIndex &parent ) const
{
	if( column > 1 )
	{
		return( QModelIndex() );
	}

	ObjectId		ParentId = OBJECT_NONE;

	if( parent.isValid() )
	{
		ParentId = ObjectId( parent.internalId() );
	}

	ObjectIdVector	ChildVec = ObjectManager::instance()->children( ParentId );

	if( ChildVec.size() <= row )
	{
		return( QModelIndex() );
	}

	return( createIndex( row, column, ChildVec.at( row )  ) );
}

QModelIndex MooItemModel::parent( const QModelIndex &child ) const
{
	if( !child.isValid() )
	{
		return( QModelIndex() );
	}

	ObjectId	ChildId = child.internalId();

	Object		*C = ObjectManager::o( ChildId );

	if( !C )
	{
		return( QModelIndex() );
	}

	ObjectId	ParentId = C->parent();

	if( ParentId == OBJECT_NONE )
	{
		return( QModelIndex() );
	}

	Object		*P = ObjectManager::o( ParentId );

	ObjectIdVector	PVec = ObjectManager::instance()->children( P ? P->parent() : OBJECT_NONE );

	int			 i = PVec.indexOf( ParentId );

	return( createIndex( i, 0, ParentId ) );
}

int MooItemModel::rowCount( const QModelIndex &parent ) const
{
	if( parent.column() > 1 )
	{
		return( 0 );
	}

	ObjectId		ParentId = OBJECT_NONE;

	if( parent.isValid() )
	{
		ParentId = ObjectId( parent.internalId() );
	}

	int				ChildCount = ObjectManager::instance()->childrenCount( ParentId );

	return( ChildCount );
}

int MooItemModel::columnCount( const QModelIndex &parent ) const
{
	return( 2 );
}

QVariant MooItemModel::data( const QModelIndex &index, int role ) const
{
	if( !index.isValid() )
	{
		return( QVariant() );
	}

	if( role != Qt::DisplayRole )
	{
		return( QVariant() );
	}

	ObjectId		Id = index.internalId();

	if( !index.column() )
	{
		return( Id );
	}

	Object			*O = ObjectManager::o( Id );

	if( !O )
	{
		return( QVariant() );
	}

	return( O->name() );
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
