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
#include <QVector>

#include "object.h"
#include "task.h"
#include "taskentry.h"

class ODB;

typedef QMap<ObjectId,Object*>	ObjectMap;
typedef QList<Object *>         ObjectList;
typedef QList<ObjectId>         ObjectIdList;
typedef QVector<ObjectId>		ObjectIdVector;

typedef struct ObjectManagerData
{
	ObjectId				 mObjNum;
	ObjectMap				 mObjMap;

	mutable QMutex			 mTaskMutex;

	QList<TaskEntry>		 mTaskList;

} ObjectManagerData;

typedef struct ObjectManagerStats
{
	int			mObjectCount;
	int			mTasks;
	int			mReads;
	int			mWrites;
	qint64		mExecutionTime;
	qint64		mCompilationTime;
} ObjectManagerStats;

class ObjectManager : public QObject
{
	Q_OBJECT

	explicit ObjectManager( QObject *parent = Q_NULLPTR );

	friend class ODB;

public:
	static ObjectManager *instance( void );

	inline static qint64 timestamp( void )
	{
		return( mTimeStamp );
	}

	inline static Object *o( ObjectId pId )
	{
		return( instance()->object( pId ) );
	}

	static void reset( void );

public:
	ObjectId newObjectId( void );

	Object *newObject( void );

	Object *object( ObjectId pIndex );

	Object *objectIncludingRecycled( ObjectId pIndex );

	void clear( void );

	ObjectId findPlayer( QString pName ) const;
	ObjectId findByProp( QString pName, QVariant pValue ) const;

	inline size_t objectCount( void ) const
	{
		return( mData.mObjMap.size() );
	}

	inline ObjectId maxId( void ) const
	{
		return( mData.mObjNum );
	}

	void luaMinimal( void );

	QList<Object *> connectedPlayers( void ) const;

	inline ODB *odb( void )
	{
		return( mODB );
	}

	inline const ODB *odb( void ) const
	{
		return( mODB );
	}

	qint64 timeToNextTask( void ) const;

	ObjectIdVector children( ObjectId pParentId ) const;

	int childrenCount( ObjectId pParentId ) const;

	QMap<ObjectId,QString> objectNames( ObjectIdVector pIds ) const;

	QString objectName( ObjectId pId ) const;

	ObjectId objectParent( ObjectId pId ) const;

signals:
	void stats( const ObjectManagerStats &pStats );
	void taskReady( void );

public slots:
	void onFrame( qint64 pTimeStamp );
	void doTask( TaskEntry &pTask );
	void queueTask( TaskEntry &pTask );
	bool killTask( TaskId pTaskId );

	void checkpoint( void );

	void setODB( ODB *pODB )
	{
		mODB = pODB;
	}

	void recycle( ObjectId pObjectId );
	void recycle( Object *pObject );

	void restore( ObjectId pObjectId );
	void restore( Object *pObject );

	void recycleObjects( void );

	void updateObject( Object *pObject );

	void addVerb( Object *pObject, QString pName );
	void deleteVerb( Object *pObject, QString pName );
	void updateVerb( Object *pObject, QString pName );
	void addProperty( Object *pObject, QString pName );
	void deleteProperty( Object *pObject, QString pName );
	void updateProperty( Object *pObject, QString pName );

	void recordRead( void )
	{
		mStats.mReads++;
	}

	void recordWrite( void )
	{
		mStats.mWrites++;
	}

	void recordExecutionTime( qint64 pMillis )
	{
		mStats.mExecutionTime += pMillis;
	}

	void recordCompilationTime( qint64 pMillis )
	{
		mStats.mCompilationTime += pMillis;
	}

	void networkRequestFinished( void );
	void networkRequestReadyRead( void );

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
	static ObjectManager		*mInstance;
	static qint64				 mTimeStamp;

	typedef struct NetRepEnt
	{
		ObjectId				 mObjectId;
		QString					 mVerb;
		QByteArray				 mData;
	} NetRepEnt;

	QList<NetRepEnt>			 mNetRepLst;

	ODB							*mODB;
	ObjectManagerData			 mData;

	ObjectManagerStats			 mStats;

	ObjectIdList				 mAddedObjects;
	ObjectIdList				 mDeletedObjects;
	ObjectIdList				 mUpdatedObjects;

	QMap<ObjectId,QStringList>	 mAddedVerbs;
	QMap<ObjectId,QStringList>	 mDeletedVerbs;
	QMap<ObjectId,QStringList>	 mUpdatedVerbs;

	QMap<ObjectId,QStringList>	 mAddedProperties;
	QMap<ObjectId,QStringList>	 mDeletedProperties;
	QMap<ObjectId,QStringList>	 mUpdatedProperties;
};

#endif // OBJECTMANAGER_H
