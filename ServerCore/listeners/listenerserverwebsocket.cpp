#include "listenerserverwebsocket.h"

#include "listenersocketwebsocket.h"

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
		}
	}
}
