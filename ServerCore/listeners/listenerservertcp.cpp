#include "listenerservertcp.h"

#include "listenersockettcp.h"

ListenerServerTCP::ListenerServerTCP( ObjectId pObjectId, quint16 pPort, QObject *pParent ) :
	ListenerServer( pObjectId, pParent )
{
	connect( &mServer, &SslServer::newConnection, this, &ListenerServerTCP::newConnection );

	mServer.listen( QHostAddress::Any, pPort );
}

void ListenerServerTCP::newConnection( void )
{
	QTcpSocket		*S;

	while( ( S = mServer.nextPendingConnection() ) != nullptr )
	{
		QSslSocket	*SS = qobject_cast<QSslSocket *>( S );

		if( SS )
		{
			qInfo() << "Socket Encrypted:" << SS->isEncrypted();
		}

		ListenerSocketTCP		*LS = new ListenerSocketTCP( this, S );

		if( !LS )
		{
			S->close();

			continue;
		}
	}
}
