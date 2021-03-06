#include "connectionmanager.h"
#include "mooexception.h"
#include "connection.h"
#include "lua_task.h"
#include "listeners/listenerserver.h"
#include "listeners/listenersocket.h"

#include <QDebug>

ConnectionManager	*ConnectionManager::mInstance = 0;

ConnectionManager::ConnectionManager( QObject *pParent ) :
	QObject( pParent ), mConnectionId( 0 )
{
}

ConnectionManager::~ConnectionManager()
{
	for( ConnectionNodeMap::iterator it = mConnectionNodeMap.begin() ; it != mConnectionNodeMap.end() ; it++ )
	{
		delete it.value();
	}

	mConnectionNodeMap.clear();

	QMutexLocker		 L( &mClosedSocketMutex );

	qDeleteAll( mClosedSocketList );

	mClosedSocketList.clear();

	mInstance = nullptr;
}

ConnectionManager *ConnectionManager::instance( void )
{
	if( mInstance )
	{
		return( mInstance );
	}

	if( ( mInstance = new ConnectionManager() ) != Q_NULLPTR )
	{
		return( mInstance );
	}

	throw( mooException( E_MEMORY, "cannot create object manager" ) );

	return( Q_NULLPTR );
}

void ConnectionManager::reset()
{
	if( mInstance )
	{
		delete mInstance;

		mInstance = nullptr;
	}
}

ConnectionId ConnectionManager::doConnect( ObjectId pListenerId )
{
	const ConnectionId	 CID = ++mConnectionId;
	Connection			*CON = new Connection( CID, this );

	if( !CON )
	{
		return( 0 );
	}

	CON->setObjectId( pListenerId );

	mConnectionNodeMap[ CID ] = CON;

	return( CID );
}

void ConnectionManager::doDisconnect( ConnectionId pConnectionId )
{
	ConnectionNodeMap::iterator it = mConnectionNodeMap.find( pConnectionId );

	if( it == mConnectionNodeMap.end() )
	{
		return;
	}

	delete it.value();

	mConnectionNodeMap.erase( it );
}

void ConnectionManager::logon( ConnectionId pConnectionId, ObjectId pPlayerId )
{
	ConnectionNodeMap::iterator it = mConnectionNodeMap.find( pConnectionId );

	if( it == mConnectionNodeMap.end() )
	{
		return;
	}

	it.value()->setPlayerId( pPlayerId );
}

void ConnectionManager::logoff( ConnectionId pConnectionId )
{
	ConnectionNodeMap::iterator it = mConnectionNodeMap.find( pConnectionId );

	if( it == mConnectionNodeMap.end() )
	{
		return;
	}

	it.value()->setPlayerId( OBJECT_NONE );
}

void ConnectionManager::disconnectAll()
{
	for( ConnectionNodeMap::iterator it = mConnectionNodeMap.begin() ; it != mConnectionNodeMap.end() ; it++ )
	{
		it.value()->flush();
		it.value()->close();
	}
}

void ConnectionManager::broadcast( const QString &pMessage )
{
	for( ConnectionNodeMap::iterator it = mConnectionNodeMap.begin() ; it != mConnectionNodeMap.end() ; it++ )
	{
		it.value()->notify( pMessage );
	}
}

ConnectionId ConnectionManager::fromPlayer( ObjectId pPlayerId )
{
	for( ConnectionNodeMap::iterator it = mConnectionNodeMap.begin() ; it != mConnectionNodeMap.end() ; it++ )
	{
		if( it.value()->player() == pPlayerId )
		{
			return( it.key() );
		}
	}

	return( 0 );
}

Connection *ConnectionManager::connection( ConnectionId pConnectionId )
{
	ConnectionNodeMap::iterator it = mConnectionNodeMap.find( pConnectionId );

	if( it == mConnectionNodeMap.end() )
	{
		return( Q_NULLPTR );
	}

	return( it.value() );
}

void ConnectionManager::closeListener( ListenerSocket *pLS )
{
	QMutexLocker		 L( &mClosedSocketMutex );

	mClosedSocketList.push_back( pLS );
}

void ConnectionManager::processClosedSockets( void )
{
	QMutexLocker		 L( &mClosedSocketMutex );

	while( !mClosedSocketList.isEmpty() )
	{
		ListenerSocket		*LS = mClosedSocketList.takeFirst();

		Connection			*CON = connection( LS->connectionId() );

		if( CON && CON->player() != OBJECT_NONE )
		{
			try
			{
				lua_task::process( QString( "moo.system:user_client_disconnected( o( %1 ) )" ).arg( CON->player() ), LS->connectionId(), CON->player() );
			}
			catch( const mooException &e )
			{
				qWarning() << e.mMessage;
			}
			catch( ... )
			{

			}

			logoff( CON->id() );
		}

		LS->deleteLater();
	}
}

