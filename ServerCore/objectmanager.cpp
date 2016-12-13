#include "objectmanager.h"

#include <algorithm>

#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QModelIndex>

#include "lua_moo.h"
#include "lua_object.h"
#include "lua_task.h"
#include "task.h"
#include "mooexception.h"
#include "connection.h"
#include "connectionmanager.h"
#include "odb.h"

ObjectManager			*ObjectManager::mInstance = 0;

ObjectManager::ObjectManager( QObject *pParent ) :
	QAbstractItemModel( pParent ), mODB( 0 )
{
	mData.mObjNum = 0;
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

	Eval.setOwner( FirstRoom->id() );
	Eval.setObject( FirstRoom->id() );
	Eval.setScript( "moo.notify( moo.eval( moo.argstr ) );" );

	FirstRoom->verbAdd( "eval", Eval );
}

void ObjectManager::reset( void )
{
	if( mInstance == 0 )
	{
		return;
	}

	delete mInstance;

	mInstance = 0;
}

Object *ObjectManager::object( ObjectId pIndex ) const
{
	Object *O = mData.mObjMap.value( pIndex, 0 );

	if( !O && mODB )
	{
		O = mODB->object( pIndex );
	}

	return( O );
}

ObjectManager *ObjectManager::instance( void )
{
	if( mInstance != 0 )
	{
		return( mInstance );
	}

	if( ( mInstance = new ObjectManager() ) != 0 )
	{
		return( mInstance );
	}

	throw( mooException( E_MEMORY, "cannot create object manager" ) );

	return( 0 );
}

void ObjectManager::clear()
{
	for( ObjectMap::iterator it = mData.mObjMap.begin() ; it != mData.mObjMap.end() ; it++ )
	{
		delete it.value();
	}

	mData.mObjNum = 0;

	mData.mObjMap.clear();
	mData.mObjTop.clear();
	mData.mPlayers.clear();
	mData.mRecycled.clear();

	mData.mTaskMutex.lock();

	mData.mTaskList.clear();
	mData.mTaskQueue.clear();

	mData.mTaskMutex.unlock();
}

Object *ObjectManager::newObject( void )
{
	Object		*O = new Object();

	if( O )
	{
		O->mData.mId = newObjectId();

		mData.mObjMap[ O->id() ] = O;

		if( mODB )
		{
			mODB->registerObject( *O );
		}
	}

	return( O );
}

void ObjectManager::recycle( Object *pObject )
{
	pObject->setRecycle( true );

	mData.mRecycled.push_back( pObject->id() );
}

ObjectId ObjectManager::newObjectId( void )
{
	return( mData.mObjNum++ );
}

void ObjectManager::recycleObjects( void )
{
	if( mData.mRecycled.empty() )
	{
		return;
	}

	foreach( ObjectId id, mData.mRecycled )
	{
		Object		*O = object( id );

		Q_ASSERT( O->recycle() );

		mData.mObjMap.remove( id );

		Q_ASSERT( O->mData.mChildren.empty() );

		delete O;
	}

	mData.mRecycled.clear();
}

void ObjectManager::addPlayer( Object *pPlayer )
{
	for( ObjectList::iterator it = mData.mPlayers.begin() ; it != mData.mPlayers.end() ; it++ )
	{
		if( *it == pPlayer )
		{
			return;
		}
	}

	mData.mPlayers.append( pPlayer );
}

void ObjectManager::remPlayer( Object *pPlayer )
{
	for( ObjectList::iterator it = mData.mPlayers.begin() ; it != mData.mPlayers.end() ; it++ )
	{
		if( *it == pPlayer )
		{
			mData.mPlayers.erase( it );
		}
	}
}

void ObjectManager::topAdd( Object *pTop )
{
	mData.mObjTop.removeAll( pTop );
	mData.mObjTop.push_back( pTop );
}

void ObjectManager::topRem( Object *pTop )
{
	mData.mObjTop.removeAll( pTop );
}

void ObjectManager::onFrame( qint64 pTimeStamp )
{
	QList<TaskEntry>		TaskList;

	mData.mTaskMutex.lock();

	TaskList.swap( mData.mTaskList );

	// Take all the queued tasks that are due to be executed and put them
	// on the current task queue

	while( !mData.mTaskQueue.empty() && mData.mTaskQueue.front().timestamp() <= pTimeStamp )
	{
		TaskList.append( mData.mTaskQueue.takeFirst() );
	}

	// Switch over the current task list

	mData.mTaskMutex.unlock();

	// Execute the current tasks

	for( const TaskEntry &TE : TaskList )
	{
		Task				 T( TE );

		qDebug() << TE.connectionid() << T.command();

		lua_task			 L( TE.connectionid(), T );

		L.execute( pTimeStamp );
	}

	TaskList.clear();

	// Recycle objects

	recycleObjects();

	// Process closed connections

	ConnectionManager::instance()->processClosedSockets();
}

void ObjectManager::doTask( TaskEntry &pTask )
{
	QMutexLocker		L( &mData.mTaskMutex );

	mData.mTaskList.push_back( pTask );
}

void ObjectManager::queueTask( TaskEntry &pTask )
{
	QMutexLocker		L( &mData.mTaskMutex );

	for( QList<TaskEntry>::iterator it = mData.mTaskQueue.begin() ; it != mData.mTaskQueue.end() ; it++ )
	{
		if( pTask.timestamp() < it->timestamp() )
		{
			mData.mTaskQueue.insert( it, pTask );

			return;
		}
	}

	mData.mTaskQueue.append( pTask );
}

bool ObjectManager::killTask(TaskId pTaskId)
{
	QMutexLocker		L( &mData.mTaskMutex );

	for( QList<TaskEntry>::iterator it = mData.mTaskQueue.begin() ; it != mData.mTaskQueue.end() ; it++ )
	{
		if( it->id() == pTaskId )
		{
			mData.mTaskQueue.erase( it );

			return( true );
		}
	}

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

QModelIndex ObjectManager::index( int row, int column, const QModelIndex &parent ) const
{
	ObjectId	ParentId = ( parent.isValid() ? reinterpret_cast<Object *>( parent.internalPointer() )->id() : OBJECT_NONE );

	if( ParentId != OBJECT_NONE )
	{
		return( createIndex( row, column, object( object( ParentId )->children().at( row ) ) ) );
	}

	return( createIndex( row, column, mData.mObjTop.at( row ) ) );
}

QModelIndex ObjectManager::parent(const QModelIndex &index) const
{
	Object		*O = reinterpret_cast<Object *>( index.internalPointer() );

	if( O->parent() == OBJECT_NONE )
	{
		return( QModelIndex() );
	}

	Object		*P = mData.mObjMap.value( O->parent(), 0 );

	if( P == 0 )
	{
		return( QModelIndex() );
	}

	Object		*PP = mData.mObjMap.value( P->parent(), 0 );

	if( PP != 0 )
	{
		return( createIndex( PP->children().indexOf( P->id() ), 0, P ) );
	}

	return( createIndex( mData.mObjTop.indexOf( P ), 0, P ) );
}

int ObjectManager::rowCount(const QModelIndex &parent) const
{
	ObjectId	ParentId = ( parent.isValid() ? reinterpret_cast<Object *>( parent.internalPointer() )->id() : OBJECT_NONE );

	if( ParentId != OBJECT_NONE )
	{
		return( object( ParentId )->children().size() );
	}

	return( mData.mObjTop.size() );
}

int ObjectManager::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED( parent )

	return( 2 );
}

QVariant ObjectManager::data(const QModelIndex &index, int role) const
{
	Object		*O = reinterpret_cast<Object *>( index.internalPointer() );

	if( role == Qt::DisplayRole )
	{
		if( index.column() == 0 )
		{
			return( O->id() );
		}

		return( O->name() );
	}

	return( QVariant() );
}
