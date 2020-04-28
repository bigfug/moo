#include "listenerserverwebsocket.h"

#include "listenersocketwebsocket.h"

#include "connectionmanager.h"
#include "objectmanager.h"

ListenerServerWebSocket::ListenerServerWebSocket( ObjectId pObjectId, quint16 pPort, QObject *pParent ) :
	ListenerServer( pObjectId, pParent ), mServer( QStringLiteral( "ArtMOO" ), QWebSocketServer::NonSecureMode, this )
{
	connect( &mServer, &QWebSocketServer::newConnection, this, &ListenerServerWebSocket::newConnection );

	mServer.listen( QHostAddress::Any, pPort );
}

void ListenerServerWebSocket::newConnection( void )
{
	QWebSocket		*S;

	while( ( S = mServer.nextPendingConnection() ) != 0 )
	{
		ListenerSocketWebSocket		*LS = new ListenerSocketWebSocket( this, S );

		if( !LS )
		{
			S->close();

			continue;
		}

		ConnectionManager	*CM = ConnectionManager::instance();

		ConnectionId		 CID = CM->doConnect( objectid() );
		Connection			*CON = CM->connection( CID );

		LS->setConnectionId( CID );

		connect( CON, &Connection::textOutput, LS, &ListenerSocketWebSocket::textInput );

		connect( CON, &Connection::taskOutput, ObjectManager::instance(), &ObjectManager::doTask );

		connect( CON, &Connection::gmcpOutput, LS, &ListenerSocketWebSocket::sendGMCP );

		connect( CON, &Connection::connectionClosed, LS, &ListenerSocketWebSocket::close );

		connect( CON, &Connection::lineModeChanged, LS, &ListenerSocketWebSocket::setLineMode );

		connect( CON, &Connection::connectionFlush, LS, &ListenerSocketWebSocket::flush );


		connect( LS, &ListenerSocketWebSocket::textOutput, CON, &Connection::dataInput );

		connect( LS, &ListenerSocketWebSocket::lineModeSupported, CON, &Connection::setLineModeSupport );

		connect( LS, &ListenerSocketWebSocket::terminalSizeChanged, CON, &Connection::setTerminalSize );

		connect( LS, &ListenerSocketWebSocket::disconnected, CM, &ConnectionManager::closeListener );

		LS->start();

		LS->setLineMode( CON->lineMode() );
	}
}
