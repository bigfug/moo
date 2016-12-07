#include "objectmanager.h"
#include <QFile>
#include <QDir>
#include <QDateTime>
#include "lua_moo.h"
#include "lua_object.h"
#include "lua_task.h"
#include "task.h"
#include <algorithm>
#include "mooexception.h"
#include "connection.h"
#include "connectionmanager.h"
#include <QModelIndex>

ObjectManager			*ObjectManager::mInstance = 0;

ObjectManager::ObjectManager( QObject *pParent ) :
	QAbstractItemModel( pParent ), mObjNum( 0 ), mTaskListId( 1 )
{
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

	Login.setOwner( System->id() );
	Login.setObject( System->id() );
	Login.setScript( QString( "return( o( %1 ) )" ).arg( Wizard->id() ) );
	Login.setDirectObjectArgument( Verb::THIS );
	Login.setPrepositionArgument( Verb::NONE );
	Login.setIndirectObjectArgument( Verb::THIS );

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

void ObjectManager::load( const QString &pDataFileName )
{
	QFile		File( pDataFileName );

	if( !File.open( QIODevice::ReadOnly ) )
	{
		return;
	}

	QDataStream		Data( &File );

	Data >> mObjNum;

	qint32			c;

	Data >> c;

	for( qint32 i = 0 ; i < c ; i++ )
	{
		Object		*O = new Object();

		O->load( Data );

		mObjMap[ O->id() ] = O;

		if( O->parent() == OBJECT_NONE )
		{
			mObjTop.push_back( O );
		}

		if( O->player() )
		{
			mPlayers.push_back( O );
		}
	}

	// Load Tasks

	qint32			TaskCount;

	Data >> TaskCount;

	for( qint32 i = 0 ; i < TaskCount ; i++ )
	{
		TaskEntry		E;

		E.load( Data );

		mTaskList1.push_back( E );
	}

	qSort( mTaskList1.begin(), mTaskList1.end(), TaskEntry::lessThan );
}

void ObjectManager::save( const QString &pDataFileName )
{
	QString		DatStr = QDateTime::currentDateTime().toString( "yyyy-MM-dd.hh-mm-ss" );
	QString		DatNam = pDataFileName.mid( pDataFileName.lastIndexOf( '\\' ) + 1 );
	QString		NamPrt = DatNam.left( DatNam.lastIndexOf( '.' ) );
	QString		NamExt = DatNam.mid( DatNam.lastIndexOf( '.' ) + 1 );
	QString		NewNam = QString( "%1.%2.tmp" ).arg( NamPrt ).arg( DatStr );
	QString		OldNam = QString( "%1.%2.%3" ).arg( NamPrt ).arg( DatStr ).arg( NamExt );

	QFile		File( NewNam );

	if( !File.open( QIODevice::WriteOnly ) )
	{
		return;
	}

	QDataStream		Data( &File );

	Data << mObjNum;

	qint32			c = mObjMap.size();

	Data << c;

	for( ObjectMap::iterator it = mObjMap.begin() ; it != mObjMap.end() ; it++ )
	{
		it.value()->save( Data );
	}

	// Save Tasks

	qint32			TaskCount = mTaskList1.size() + mTaskList2.size();

	Data << TaskCount;

	for( QList<TaskEntry>::const_iterator it = mTaskList1.begin() ; it != mTaskList1.end() ; it++ )
	{
		it->save( Data );
	}

	for( QList<TaskEntry>::const_iterator it = mTaskList2.begin() ; it != mTaskList2.end() ; it++ )
	{
		it->save( Data );
	}

	File.close();

	QDir().rename( pDataFileName, OldNam );
	QDir().rename( NewNam, pDataFileName );
}

void ObjectManager::clear()
{
	for( ObjectMap::iterator it = mObjMap.begin() ; it != mObjMap.end() ; it++ )
	{
		delete it.value();
	}
}

Object *ObjectManager::newObject( void )
{
	Object		*O = new Object();

	if( O == 0 )
	{
		return( 0 );
	}

	O->mId = newObjectId();

	mObjMap[ O->id() ] = O;

	return( O );
}

void ObjectManager::recycle( Object *pObject )
{
	pObject->setRecycle( true );

	mRecycled.push_back( pObject->id() );
}

ObjectId ObjectManager::newObjectId( void )
{
	return( mObjNum++ );
}

void ObjectManager::recycleObjects( void )
{
	if( mRecycled.empty() )
	{
		return;
	}

	foreach( ObjectId id, mRecycled )
	{
		Object		*O = object( id );

		Q_ASSERT( O->recycle() );

		mObjMap.remove( id );

		Q_ASSERT( O->mChildren.empty() );

		delete O;
	}

	mRecycled.clear();
}

void ObjectManager::addPlayer( Object *pPlayer )
{
	for( ObjectList::iterator it = mPlayers.begin() ; it != mPlayers.end() ; it++ )
	{
		if( *it == pPlayer )
		{
			return;
		}
	}

	mPlayers.append( pPlayer );
}

void ObjectManager::remPlayer( Object *pPlayer )
{
	for( ObjectList::iterator it = mPlayers.begin() ; it != mPlayers.end() ; it++ )
	{
		if( *it == pPlayer )
		{
			mPlayers.erase( it );
		}
	}
}

void ObjectManager::topAdd( Object *pTop )
{
	mObjTop.removeAll( pTop );
	mObjTop.push_back( pTop );
}

void ObjectManager::topRem( Object *pTop )
{
	mObjTop.removeAll( pTop );
}

void ObjectManager::onFrame( qint64 pTimeStamp )
{
	QList<TaskEntry>		&TaskList  = ( mTaskListId == 1 ? mTaskList1 : mTaskList2 );

	mTaskMutex.lock();

	// Take all the queued tasks that are due to be executed and put them
	// on the current task queue

	while( !mTaskQueue.empty() && mTaskQueue.front().timestamp() <= pTimeStamp )
	{
		TaskList.append( mTaskQueue.takeFirst() );
	}

	// Switch over the current task list

	mTaskListId = ( mTaskListId == 1 ? 2 : 1 );

	mTaskMutex.unlock();

	// Execute the current tasks

	for( QList<TaskEntry>::iterator it = TaskList.begin() ; it != TaskList.end() ; it++ )
	{
		const TaskEntry		&TE = *it;

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
	QMutexLocker		L( &mTaskMutex );

	if( mTaskListId == 1 )
	{
		mTaskList1.push_back( pTask );
	}
	else
	{
		mTaskList2.push_back( pTask );
	}
}

void ObjectManager::queueTask( TaskEntry &pTask )
{
	QMutexLocker		L( &mTaskMutex );

	for( QList<TaskEntry>::iterator it = mTaskQueue.begin() ; it != mTaskQueue.end() ; it++ )
	{
		if( pTask.timestamp() < it->timestamp() )
		{
			mTaskQueue.insert( it, pTask );

			return;
		}
	}

	mTaskQueue.append( pTask );
}

bool ObjectManager::killTask(TaskId pTaskId)
{
	QMutexLocker		L( &mTaskMutex );

	for( QList<TaskEntry>::iterator it = mTaskQueue.begin() ; it != mTaskQueue.end() ; it++ )
	{
		if( it->id() == pTaskId )
		{
			mTaskQueue.erase( it );

			return( true );
		}
	}

	QList<TaskEntry>		&TaskList  = ( mTaskListId == 1 ? mTaskList1 : mTaskList2 );

	for( QList<TaskEntry>::iterator it = TaskList.begin() ; it != TaskList.end() ; it++ )
	{
		if( it->id() == pTaskId )
		{
			TaskList.erase( it );

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

	return( createIndex( row, column, mObjTop.at( row ) ) );
}

QModelIndex ObjectManager::parent(const QModelIndex &index) const
{
	Object		*O = reinterpret_cast<Object *>( index.internalPointer() );

	if( O->parent() == OBJECT_NONE )
	{
		return( QModelIndex() );
	}

	Object		*P = mObjMap.value( O->parent(), 0 );

	if( P == 0 )
	{
		return( QModelIndex() );
	}

	Object		*PP = mObjMap.value( P->parent(), 0 );

	if( PP != 0 )
	{
		return( createIndex( PP->children().indexOf( P->id() ), 0, P ) );
	}

	return( createIndex( mObjTop.indexOf( P ), 0, P ) );
}

int ObjectManager::rowCount(const QModelIndex &parent) const
{
	ObjectId	ParentId = ( parent.isValid() ? reinterpret_cast<Object *>( parent.internalPointer() )->id() : OBJECT_NONE );

	if( ParentId != OBJECT_NONE )
	{
		return( object( ParentId )->children().size() );
	}

	return( mObjTop.size() );
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
