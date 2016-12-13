//! ObjectManager
/*!
asd
*/

#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include <QObject>
#include <QMap>
#include <QList>
#include <QMutex>
#include <QAbstractItemModel>

#include "object.h"
#include "task.h"
#include "taskentry.h"

class ODB;

typedef QMap<ObjectId,Object*>	ObjectMap;
typedef QList<Object *>         ObjectList;
typedef QList<ObjectId>         ObjectIdList;

typedef struct ObjectManagerData
{
	ObjectId				 mObjNum;
	ObjectMap				 mObjMap;
	ObjectList				 mObjTop;
	ObjectList				 mPlayers;
	ObjectIdList			 mRecycled;

	mutable QMutex			 mTaskMutex;

	QList<TaskEntry>		 mTaskList;
	QList<TaskEntry>		 mTaskQueue;

} ObjectManagerData;

class ObjectManager : public QAbstractItemModel
{
	Q_OBJECT

	explicit ObjectManager( QObject *parent = 0 );

	friend class ODB;

public:
	static ObjectManager *instance( void );

	ObjectId newObjectId( void );

	Object *newObject( void );

	void clear( void );

	static void reset( void );

	inline static Object *o( ObjectId pId )
	{
		return( instance()->object( pId ) );
	}

	Object *object( ObjectId pIndex ) const;

	void recycle( Object *pObject );

	void recycleObjects( void );

	inline size_t objectCount( void ) const
	{
		return( mData.mObjMap.size() );
	}

	inline ObjectId maxId( void ) const
	{
		return( mData.mObjNum );
	}

	//! returns the list of active players

	inline const ObjectList &players( void ) const
	{
		return( mData.mPlayers );
	}

	void addPlayer( Object *pPlayer );
	void remPlayer( Object *pPlayer );

	void topAdd( Object *pTop );
	void topRem( Object *pTop );

	void luaMinimal( void );

signals:

public slots:
	void onFrame( qint64 pTimeStamp );
	void doTask( TaskEntry &pTask );
	void queueTask( TaskEntry &pTask );
	bool killTask( TaskId pTaskId );

private:
	// QAbstractItemModel

	virtual QModelIndex index( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
	virtual QModelIndex parent ( const QModelIndex & index ) const;
	virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
	virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
	virtual QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;

protected:
	ObjectManagerData &data( void )
	{
		return( mData );
	}

	const ObjectManagerData &data( void ) const
	{
		return( mData );
	}

private:
	static ObjectManager	*mInstance;

	ODB						*mODB;
	ObjectManagerData		 mData;
};

#endif // OBJECTMANAGER_H
