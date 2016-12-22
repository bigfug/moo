#include "listenerwebsocket.h"

#include "listenerwebsocketsocket.h"

ListenerWebSocket::ListenerWebSocket( ObjectId pObjectId, quint16 pPort, QObject *pParent ) :
	ListenerServer( pObjectId, pParent ), mServer( QStringLiteral( "ArtMOO" ), QWebSocketServer::NonSecureMode, this )
{
	connect( &mServer, &QWebSocketServer::newConnection, this, &ListenerWebSocket::newConnection );

	mServer.listen( QHostAddress::Any, pPort );
}

void ListenerWebSocket::newConnection( void )
{
	QWebSocket		*S;

	while( ( S = mServer.nextPendingConnection() ) != 0 )
	{
		ListenerWebSocketSocket		*LS = new ListenerWebSocketSocket( this, S );

		if( !LS )
		{
			S->close();
		}
	}
}
