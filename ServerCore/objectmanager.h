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

typedef struct TransferInformation
{
	int			mObjects;
	int			mVerbs;
	int			mProperties;
	int			mMilliseconds;

	TransferInformation( void )
		: mObjects( 0 ), mVerbs( 0 ), mProperties( 0 ), mMilliseconds( 0 )
	{

	}
} TransferInformation;

class ObjectManager : public QObject
{
	Q_OBJECT

	explicit ObjectManager( QObject *parent = Q_NULLPTR );

	virtual ~ObjectManager( void );

	friend class ODB;

public:
	static constexpr ObjectId	TemporaryObjectIdStart = -100;

	static ObjectManager *instance( void );

	inline static qint64 timestamp( void )
	{
		return( mTimeStamp );
	}

	inline static Object *o( ObjectId pId )
	{
		return( instance()->object( pId ) );
	}

	static bool isTemporaryObjectId( ObjectId pId )
	{
		return( pId <= TemporaryObjectIdStart );
	}

	static void reset( void );

public:
	ObjectId newObjectId( void );

	ObjectId newTemporaryObjectId( void );

	Object *newObject( void );

	Object *newTemporaryObject( void );

	Object *object( ObjectId pIndex );

	Object *objectIncludingRecycled( ObjectId pIndex );

	Object *rootObject( void );
	Object *systemObject( void );

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

	ObjectIdVector connectedObjects( void ) const;

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

	void exportModule( ObjectId pModuleId, const QString &pFileName, TransferInformation &pTrnInf ) const;

	ObjectId importModule( ObjectId pParentId, ObjectId pOwnerId, const QString &pFileName, TransferInformation &pTrnInf );

	void objectConnect( ObjectId pSrcObj, QString pSrcVrb, ObjectId pDstObj, QString pDstVrb );

	void objectDisconnect( ObjectId pSrcObj, QString pSrcVrb = QString(), ObjectId pDstObj = OBJECT_NONE, QString pDstVrb = QString() );

	QVector<QPair<ObjectId,QString>> objectSignals( ObjectId pSrcObj, QString pSrcVrb );

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
	void addSignalConnection( const SignalConnection &pSigCon );
	void deleteSignalConnection( const SignalConnection &pSigCon );

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

	int							 mTempObjectId;

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

	QVector<SignalConnection>	 mAddedConnections;
	QVector<SignalConnection>	 mDeletedConnections;

	Q_DISABLE_COPY( ObjectManager )
};

#endif // OBJECTMANAGER_H
