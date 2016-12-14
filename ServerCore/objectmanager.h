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

class ODB;

typedef QMap<ObjectId,Object*>	ObjectMap;
typedef QList<Object *>         ObjectList;
typedef QList<ObjectId>         ObjectIdList;

typedef struct ObjectManagerData
{
	ObjectId				 mObjNum;
	ObjectMap				 mObjMap;
	ObjectIdList			 mRecycled;

	mutable QMutex			 mTaskMutex;

	QList<TaskEntry>		 mTaskList;
	QList<TaskEntry>		 mTaskQueue;

} ObjectManagerData;

class ObjectManager : public QObject
{
	Q_OBJECT

	explicit ObjectManager( QObject *parent = 0 );

	friend class ODB;

public:
	static ObjectManager *instance( void );

	inline static qint64 timestamp( void )
	{
		return( mTimeStamp );
	}

	ObjectId newObjectId( void );

	Object *newObject( void );

	void clear( void );

	static void reset( void );

	void markObject( ObjectId pIndex );
	void markObject( Object *O );

	ObjectId findPlayer( QString pName ) const;

	void setODB( ODB *pODB )
	{
		mODB = pODB;
	}

	inline static Object *o( ObjectId pId )
	{
		return( instance()->object( pId ) );
	}

	Object *object( ObjectId pIndex );

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

	void luaMinimal( void );

signals:

public slots:
	void onFrame( qint64 pTimeStamp );
	void doTask( TaskEntry &pTask );
	void queueTask( TaskEntry &pTask );
	bool killTask( TaskId pTaskId );

protected:
	void timeoutObjects( void );

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
	static qint64			 mTimeStamp;

	ODB						*mODB;
	ObjectManagerData		 mData;
};

#endif // OBJECTMANAGER_H
