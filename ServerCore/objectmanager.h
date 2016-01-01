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
#include "object.h"
#include "task.h"
#include "taskentry.h"
#include <QAbstractItemModel>

class ObjectManager : public QAbstractItemModel
{
	Q_OBJECT

	explicit ObjectManager( QObject *parent = 0 );

public:
	typedef QMap<ObjectId,Object*>	ObjectMap;
	typedef QList<Object *>         ObjectList;
	typedef QList<ObjectId>         ObjectIdList;

	static ObjectManager *instance( void );

	ObjectId newObjectId( void );

	Object *newObject( void );

	void load( const QString &pDataFileName );
	void save( const QString &pDataFileName );
	void clear( void );

	static void reset( void );

	inline static Object *o( ObjectId pId )
	{
		return( instance()->object( pId ) );
	}

	inline Object *object( ObjectId pIndex ) const
	{
		return( mObjMap.value( pIndex, 0 ) );
	}

	void recycle( Object *pObject );

	void recycleObjects( void );

	inline size_t objectCount( void ) const
	{
		return( mObjMap.size() );
	}

	inline ObjectId maxId( void ) const
	{
		return( mObjNum );
	}

	//! returns the list of active players

	inline const ObjectList &players( void ) const
	{
		return( mPlayers );
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


private:
	static ObjectManager	*mInstance;

	ObjectId				 mObjNum;
	ObjectMap				 mObjMap;
	ObjectList				 mObjTop;
	ObjectList				 mPlayers;
	ObjectIdList			 mRecycled;

	QMutex					 mTaskMutex;
	int						 mTaskListId;
	QList<TaskEntry>		 mTaskList1;
	QList<TaskEntry>		 mTaskList2;
	QList<TaskEntry>		 mTaskQueue;
};

#endif // OBJECTMANAGER_H
