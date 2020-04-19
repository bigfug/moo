#include "objectmanager.h"

#include <algorithm>

#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QModelIndex>
#include <QNetworkReply>

#include "lua_moo.h"
#include "lua_object.h"
#include "lua_task.h"
#include "task.h"
#include "mooexception.h"
#include "connection.h"
#include "connectionmanager.h"
#include "odb.h"
#include "odb_file.h"

#include "changeset/connectionnotify.h"

ObjectManager			*ObjectManager::mInstance  = Q_NULLPTR;
qint64					 ObjectManager::mTimeStamp = 0;

ObjectManager::ObjectManager( QObject *pParent )
	: QObject( pParent ), mODB( Q_NULLPTR ), mTempObjectId( TemporaryObjectIdStart )
{
	mData.mObjNum = 0;

	memset( &mStats, 0, sizeof( mStats ) );

	mTimeStamp = QDateTime::currentMSecsSinceEpoch();
}

ObjectManager::~ObjectManager()
{
	clear();
}

// The database is empty so set up the most minimal (but useful) set of objects
// Based on LambdaMOO minimal db
// https://github.com/wrog/lambdamoo/blob/master/README.Minimal

void ObjectManager::luaMinimal( void )
{
	Object			*System		= newObject();
	Object			*Root		= newObject();
	Object			*FirstRoom	= newObject();
	Object			*Wizard		= newObject();

	// Object #0 is The System Object

	System->setName( "System Object" );
	System->setParent( Root->id() );

	// Object #1 is The Root Class. All objects are descended from Object #1

	Root->setName( "Root Class" );

	// Object #2 is The First Room - it has the basic 'eval' verb defined

	FirstRoom->setName( "The First Room" );
	FirstRoom->setParent( Root->id() );

	// Object #3 is the only player object

	Wizard->setName( "Wizard" );
	Wizard->setParent( Root->id() );
	Wizard->setProgrammer( true );
	Wizard->setWizard( true );
	Wizard->setPlayer( true );
	Wizard->move( FirstRoom );

	// Initialise the login verb to allow the player to login to the system

	Verb			Login;

	Login.initialise();

	Login.setOwner( Wizard->id() );
	Login.setObject( System->id() );
	Login.setScript( QString( "return( o( %1 ) )" ).arg( Wizard->id() ) );
	Login.setDirectObjectArgument( THIS );
	Login.setPrepositionArgument( NONE );
	Login.setIndirectObjectArgument( THIS );

	System->verbAdd( "do_login_command", Login );

	// Define the most basic eval verb to allow further programming

	Verb			Eval;

	Eval.initialise();

	Eval.setOwner( Wizard->id() );
	Eval.setObject( FirstRoom->id() );
	Eval.setScript( "moo.programmer = moo.player;\n\nmoo.notify( moo.eval( moo.argstr ) );" );

	FirstRoom->verbAdd( "eval", Eval );
}

QList<Object *> ObjectManager::connectedPlayers() const
{
	QList<Object *>		ObjLst;

	for( QMap<ObjectId,Object*>::const_iterator it = mData.mObjMap.constBegin() ; it != mData.mObjMap.constEnd() ; it++ )
	{
		Object		*O = it.value();

		if( !O->player() || O->connection() == CONNECTION_NONE )
		{
			continue;
		}

		ObjLst << O;
	}

	return( ObjLst );
}

qint64 ObjectManager::timeToNextTask() const
{
	qint64			NextTaskTime = ( mODB ? mODB->nextTaskTime() : -1 );

	if( NextTaskTime != -1 )
	{
		NextTaskTime = NextTaskTime - QDateTime::currentMSecsSinceEpoch();
	}

	return( NextTaskTime );
}

ObjectIdVector ObjectManager::children( ObjectId pParentId ) const
{
	if( isTemporaryObjectId( pParentId ) )
	{
		return( ObjectIdVector() );
	}

	return( mODB ? mODB->children( pParentId ) : ObjectIdVector() );
}

int ObjectManager::childrenCount( ObjectId pParentId ) const
{
	if( isTemporaryObjectId( pParentId ) )
	{
		return( 0 );
	}

	return( mODB ? mODB->childrenCount( pParentId ) : 0 );
}

QMap<ObjectId,QString> ObjectManager::objectNames( ObjectIdVector pIds ) const
{
	QMap<ObjectId,QString>		ObjNam;
	ObjectIdVector				ObjIds;

	for( ObjectId OID : pIds )
	{
		if( isTemporaryObjectId( OID ) )
		{
			continue;
		}

		Object *O = mData.mObjMap.value( OID, Q_NULLPTR );

		if( O )
		{
			ObjNam.insert( O->id(), O->name() );
		}
		else
		{
			ObjIds << OID;
		}
	}

	if( ObjIds.isEmpty() )
	{
		return( ObjNam );
	}

	QMap<ObjectId,QString>		ObjRet;

	if( mODB )
	{
		ObjRet = mODB->objectNames( ObjIds );
	}

	if( !ObjNam.isEmpty() )
	{
		return( ObjNam.unite( ObjRet ) );
	}

	return( ObjRet );
}

QString ObjectManager::objectName( ObjectId pId ) const
{
	if( isTemporaryObjectId( pId ) )
	{
		return( QString() );
	}

	Object *O = mData.mObjMap.value( pId, Q_NULLPTR );

	if( O )
	{
		return( O->name() );
	}

	return( mODB ? mODB->objectName( pId ) : QString() );
}

ObjectId ObjectManager::objectParent( ObjectId pId ) const
{
	if( isTemporaryObjectId( pId ) )
	{
		return( OBJECT_NONE );
	}

	Object *O = mData.mObjMap.value( pId, Q_NULLPTR );

	if( O )
	{
		return( O->parent() );
	}

	return( mODB ? mODB->objectParent( pId ) : OBJECT_NONE );
}

void ObjectManager::exportModule( ObjectId pModuleId, const QString &pFileName, TransferInformation &pTrnInf ) const
{
	if( mODB )
	{
		mODB->exportModule( pModuleId, pFileName, pTrnInf );
	}
}

ObjectId ObjectManager::importModule( ObjectId pParentId, ObjectId pOwnerId, const QString &pFileName, TransferInformation &pTrnInf )
{
	return( mODB ? mODB->importModule( pParentId, pOwnerId, pFileName, pTrnInf ) : OBJECT_NONE );
}

void ObjectManager::reset( void )
{
	if( !mInstance )
	{
		return;
	}

	delete mInstance;

	mInstance = Q_NULLPTR;
}

ObjectId ObjectManager::findPlayer( QString pName ) const
{
	return( mODB ? mODB->findPlayer( pName ) : OBJECT_NONE );
}

ObjectId ObjectManager::findByProp( QString pName, QVariant pValue ) const
{
	return( mODB ? mODB->findByProp( pName, pValue ) : OBJECT_NONE );
}

Object *ObjectManager::object( ObjectId pIndex )
{
	if( isTemporaryObjectId( pIndex ) )
	{
		return( Q_NULLPTR );
	}

	Object		*O = objectIncludingRecycled( pIndex );

	return( !O || O->recycle() ? Q_NULLPTR : O );
}

Object *ObjectManager::objectIncludingRecycled( ObjectId pIndex )
{
	if( pIndex < 0 )
	{
		return( nullptr );
	}

	Object *O = mData.mObjMap.value( pIndex, Q_NULLPTR );

	if( !O && mODB )
	{
		if( ( O = mODB->object( pIndex ) ) != Q_NULLPTR )
		{
			mData.mObjMap.insert( pIndex, O );
		}
	}

	if( O )
	{
		O->data().mLastRead = ObjectManager::timestamp();
	}

	return( O );
}

Object *ObjectManager::rootObject()
{
	return( object( 1 ) );
}

Object *ObjectManager::systemObject()
{
	return( object( 0 ) );
}

ObjectManager *ObjectManager::instance( void )
{
	if( mInstance )
	{
		return( mInstance );
	}

	if( ( mInstance = new ObjectManager() ) != Q_NULLPTR )
	{
		return( mInstance );
	}

	throw( mooException( E_MEMORY, "cannot create object manager" ) );

	return( Q_NULLPTR );
}

void ObjectManager::clear()
{
	for( ObjectMap::iterator it = mData.mObjMap.begin() ; it != mData.mObjMap.end() ; it++ )
	{
		delete it.value();
	}

	mData.mObjNum = 0;

	mData.mObjMap.clear();

	mData.mTaskMutex.lock();

	mData.mTaskList.clear();

	mData.mTaskMutex.unlock();

	mAddedObjects.clear();
	mDeletedObjects.clear();
	mUpdatedObjects.clear();
}

Object *ObjectManager::newObject( void )
{
	Object		*O = new Object();

	if( O )
	{
		O->mData.mId = newObjectId();

		mData.mObjMap[ O->id() ] = O;

		mAddedObjects << O->id();
	}

	return( O );
}

Object *ObjectManager::newTemporaryObject()
{
	Object		*O = new Object();

	if( O )
	{
		O->mData.mId = newTemporaryObjectId();
	}

	return( O );
}

void ObjectManager::recycle( Object *pObject )
{
	pObject->setRecycled( true );

	if( !isTemporaryObjectId( pObject->id() ) )
	{
		mAddedObjects.removeAll( pObject->id() );
		mUpdatedObjects.removeAll( pObject->id() );

		// TODO: Verbs, Props

		mDeletedObjects << pObject->id();
	}
}

void ObjectManager::restore( ObjectId pObjectId )
{
	restore( objectIncludingRecycled( pObjectId ) );
}

void ObjectManager::restore( Object *pObject )
{
	pObject->setRecycled( false );

	if( !isTemporaryObjectId( pObject->id() ) )
	{
		mDeletedObjects.removeAll( pObject->id() );

		if( mODB )
		{
			if( mODB->hasObject( pObject->id() ) )
			{
				mUpdatedObjects << pObject->id();
			}
			else
			{
				mAddedObjects << pObject->id();
			}
		}
	}
}

ObjectId ObjectManager::newObjectId( void )
{
	return( mData.mObjNum++ );
}

ObjectId ObjectManager::newTemporaryObjectId()
{
	return( mTempObjectId-- );
}

void ObjectManager::recycleObjects( void )
{
	if( mDeletedObjects.empty() )
	{
		return;
	}

	for( ObjectId id : mDeletedObjects )
	{
		Object		*O = objectIncludingRecycled( id );

		if( mODB )
		{
			mODB->deleteObject( *O );
		}

		Q_ASSERT( O->recycle() );

		mData.mObjMap.remove( id );

		Q_ASSERT( O->mData.mChildren.empty() );

		delete O;
	}

	mDeletedObjects.clear();
}

void ObjectManager::updateObject( Object *pObject )
{
	if( !isTemporaryObjectId( pObject->id() ) )
	{
		mUpdatedObjects.removeAll( pObject->id() );
		mUpdatedObjects.append( pObject->id() );
	}
}

void ObjectManager::addVerb( Object *pObject, QString pName )
{
	if( isTemporaryObjectId( pObject->id() ) )
	{
		return;
	}

	QStringList		StrLst = mAddedVerbs.value( pObject->id() );

	StrLst.removeAll( pName );
	StrLst.append( pName );

	mAddedVerbs.insert( pObject->id(), StrLst );
}

void ObjectManager::deleteVerb( Object *pObject, QString pName )
{
	if( isTemporaryObjectId( pObject->id() ) )
	{
		return;
	}

	QStringList		StrLst = mDeletedVerbs.value( pObject->id() );

	StrLst.removeAll( pName );
	StrLst.append( pName );

	mDeletedVerbs.insert( pObject->id(), StrLst );
}

void ObjectManager::updateVerb( Object *pObject, QString pName )
{
	if( isTemporaryObjectId( pObject->id() ) )
	{
		return;
	}

	QStringList		StrLst = mUpdatedVerbs.value( pObject->id() );

	StrLst.removeAll( pName );
	StrLst.append( pName );

	mUpdatedVerbs.insert( pObject->id(), StrLst );
}

void ObjectManager::addProperty(Object *pObject, QString pName)
{
	if( isTemporaryObjectId( pObject->id() ) )
	{
		return;
	}

	QStringList		StrLst = mAddedProperties.value( pObject->id() );

	StrLst.removeAll( pName );
	StrLst.append( pName );

	mAddedProperties.insert( pObject->id(), StrLst );
}

void ObjectManager::deleteProperty(Object *pObject, QString pName)
{
	if( isTemporaryObjectId( pObject->id() ) )
	{
		return;
	}

	QStringList		StrLst = mDeletedProperties.value( pObject->id() );

	StrLst.removeAll( pName );
	StrLst.append( pName );

	mDeletedProperties.insert( pObject->id(), StrLst );
}

void ObjectManager::updateProperty(Object *pObject, QString pName)
{
	if( isTemporaryObjectId( pObject->id() ) )
	{
		return;
	}

	QStringList		StrLst = mUpdatedProperties.value( pObject->id() );

	StrLst.removeAll( pName );
	StrLst.append( pName );

	mUpdatedProperties.insert( pObject->id(), StrLst );
}

void ObjectManager::networkRequestFinished()
{
	QNetworkReply		*NetRep = qobject_cast<QNetworkReply *>( sender() );

	if( NetRep )
	{
		NetRepEnt		NRE;

		NRE.mObjectId = NetRep->property( "oid" ).toInt();
		NRE.mVerb     = NetRep->property( "verb" ).toString();
		NRE.mData     = NetRep->readAll();

		mNetRepLst.append( NRE );
	}

	NetRep->deleteLater();
}

void ObjectManager::networkRequestReadyRead()
{

}

void ObjectManager::onFrame( qint64 pTimeStamp )
{
	mTimeStamp = QDateTime::currentMSecsSinceEpoch();

	static qint64		LastTime = mTimeStamp;

	for( const NetRepEnt &NRE : mNetRepLst )
	{
		Object		*O = object( NRE.mObjectId );
		Verb		*V = ( O ? O->verb( NRE.mVerb ) : nullptr );

		if( !V )
		{
			continue;
		}

		lua_task	 L( 0, Task() );

		lua_pushlstring( L.L(), NRE.mData.constData(), NRE.mData.size() );

		L.setPermissions( V->owner() );

		L.verbCall( NRE.mObjectId, V, 1 );

		mStats.mTasks++;
	}

	mNetRepLst.clear();

	QList<TaskEntry>		TaskList;

	mData.mTaskMutex.lock();

	// Switch over the current task list

	TaskList.swap( mData.mTaskList );

	mData.mTaskMutex.unlock();

	for( const TaskEntry &TE : TaskList )
	{
		Task				 T( TE );

		qDebug() << TE.connectionid() << T.command();

		lua_task			 L( TE.connectionid(), T );

		int		RetCnt = L.execute( pTimeStamp );

		// If we have any results from the task, print them to the connection

		if( RetCnt > 0 && TE.connectionid() )
		{
			Connection			*C = ConnectionManager::instance()->connection( TE.connectionid() );

			if( C )
			{
				for( int i = 0 ; i < RetCnt ; i++ )
				{
					QString			S;

					if( lua_isstring( L.L(), -1 ) )
					{
						size_t				 StrLen;
						const char			*StrDat = lua_tolstring( L.L(), -1, &StrLen );

						S = QString::fromLatin1( StrDat, StrLen );
					}
					else if( lua_isnumber( L.L(), -1 ) )
					{
						lua_Number			 N = lua_tonumber( L.L(), -1 );

						S = QString::number( N );
					}

					if( !S.isEmpty() )
					{
						L.changeAdd( new change::ConnectionNotify( C, S ) );
					}

					lua_pop( L.L(), 1 );
				}
			}
		}

		mStats.mTasks++;
	}

	TaskList.clear();

	// Take all the queued tasks that are due to be executed and put them
	// on the current task queue

	if( mODB )
	{
		// Execute the current tasks

		for( const TaskEntry &TE : mODB->tasks( pTimeStamp ) )
		{
			Task				 T( TE );

			qDebug() << TE.connectionid() << T.command();

			T.setObject( T.player() );

			lua_task			 L( TE.connectionid(), T );

			L.eval();

			mStats.mTasks++;
		}
	}

	// Process closed connections

	ConnectionManager::instance()->processClosedSockets();

	timeoutObjects();

	if( mTimeStamp - LastTime > 1000 )
	{
		mStats.mObjectCount = mData.mObjMap.size();

		emit stats( mStats );

		memset( &mStats, 0, sizeof( mStats ) );

		LastTime += 1000;
	}
}

void ObjectManager::doTask( TaskEntry &pTask )
{
	QMutexLocker		L( &mData.mTaskMutex );

	mData.mTaskList.push_back( pTask );

	emit taskReady();
}

void ObjectManager::queueTask( TaskEntry &pTask )
{
	if( mODB )
	{
		mODB->addTask( pTask );
	}

	emit taskReady();
}

bool ObjectManager::killTask(TaskId pTaskId)
{
	if( mODB )
	{
		mODB->killTask( pTaskId );
	}

	QMutexLocker		L( &mData.mTaskMutex );

	for( QList<TaskEntry>::iterator it = mData.mTaskList.begin() ; it != mData.mTaskList.end() ; it++ )
	{
		if( it->id() == pTaskId )
		{
			mData.mTaskList.erase( it );

			return( true );
		}
	}

	return( false );
}

void ObjectManager::checkpoint()
{
	if( mODB )
	{
		mODB->checkpoint();
	}
}

void ObjectManager::recycle(ObjectId pObjectId)
{
	recycle( object( pObjectId ) );
}

void ObjectManager::timeoutObjects()
{
	for( ObjectId OID : mAddedObjects )
	{
		Object		*O = mData.mObjMap.value( OID );

		if( O && mODB )
		{
			mODB->addObject( *O );
		}
	}

	mAddedObjects.clear();

	//-------------------------------------------------------------------------

	for( ObjectId OID : mUpdatedObjects )
	{
		Object		*O = mData.mObjMap.value( OID );

		if( O && mODB )
		{
			mODB->updateObject( *O );
		}
	}

	mUpdatedObjects.clear();

	//-------------------------------------------------------------------------

	for( QMap<ObjectId,QStringList>::const_iterator it = mAddedVerbs.begin() ; it != mAddedVerbs.end() ; it++ )
	{
		Object		*O = mData.mObjMap.value( it.key() );

		if( O && mODB )
		{
			for( QString Name : it.value() )
			{
				mODB->addVerb( *O, Name );
			}
		}
	}

	mAddedVerbs.clear();

	//-------------------------------------------------------------------------

	for( QMap<ObjectId,QStringList>::const_iterator it = mUpdatedVerbs.begin() ; it != mUpdatedVerbs.end() ; it++ )
	{
		Object		*O = mData.mObjMap.value( it.key() );

		if( O && mODB )
		{
			for( QString Name : it.value() )
			{
				mODB->updateVerb( *O, Name );
			}
		}
	}

	mUpdatedVerbs.clear();

	//-------------------------------------------------------------------------

	for( QMap<ObjectId,QStringList>::const_iterator it = mDeletedVerbs.begin() ; it != mDeletedVerbs.end() ; it++ )
	{
		Object		*O = mData.mObjMap.value( it.key() );

		if( O && mODB )
		{
			for( QString Name : it.value() )
			{
				mODB->deleteVerb( *O, Name );
			}
		}
	}

	mDeletedVerbs.clear();

	//-------------------------------------------------------------------------

	for( QMap<ObjectId,QStringList>::const_iterator it = mAddedProperties.begin() ; it != mAddedProperties.end() ; it++ )
	{
		Object		*O = mData.mObjMap.value( it.key() );

		if( O && mODB )
		{
			for( QString Name : it.value() )
			{
				mODB->addProperty( *O, Name );
			}
		}
	}

	mAddedProperties.clear();

	//-------------------------------------------------------------------------

	for( QMap<ObjectId,QStringList>::const_iterator it = mUpdatedProperties.begin() ; it != mUpdatedProperties.end() ; it++ )
	{
		Object		*O = mData.mObjMap.value( it.key() );

		if( O && mODB )
		{
			for( QString Name : it.value() )
			{
				mODB->updateProperty( *O, Name );
			}
		}
	}

	mUpdatedProperties.clear();

	//-------------------------------------------------------------------------

	for( QMap<ObjectId,QStringList>::const_iterator it = mDeletedProperties.begin() ; it != mDeletedProperties.end() ; it++ )
	{
		Object		*O = mData.mObjMap.value( it.key() );

		if( O && mODB )
		{
			for( QString Name : it.value() )
			{
				mODB->deleteProperty( *O, Name );
			}
		}
	}

	mDeletedProperties.clear();

	//-------------------------------------------------------------------------

	recycleObjects();

	//-------------------------------------------------------------------------

	ObjectList	IDS = mData.mObjMap.values();

	for( Object *O : IDS )
	{
		if( O->player() )
		{
			continue;
		}

		if( O->id() <= 5 )
		{
			continue;
		}

		if( mTimeStamp - O->data().mLastRead < 15 * 1000 )
		{
			continue;
		}

		mData.mObjMap.remove( O->id() );

		delete O;
	}
}
