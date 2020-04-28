#include "listenerservertcp.h"

#include "listenersockettcp.h"

#include "connectionmanager.h"
#include "objectmanager.h"

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

		ConnectionManager	*CM = ConnectionManager::instance();

		ConnectionId		 CID = CM->doConnect( objectid() );
		Connection			*CON = CM->connection( CID );

		LS->setConnectionId( CID );

		connect( CON, &Connection::textOutput, LS, &ListenerSocketTCP::textInput );

		connect( CON, &Connection::taskOutput, ObjectManager::instance(), &ObjectManager::doTask );

		connect( CON, &Connection::gmcpOutput, LS, &ListenerSocketTCP::sendGMCP );

		connect( CON, &Connection::connectionClosed, LS, &ListenerSocketTCP::close );

		connect( CON, &Connection::lineModeChanged, LS, &ListenerSocketTCP::setLineMode );

		connect( CON, &Connection::connectionFlush, LS, &ListenerSocketTCP::flush );


		connect( LS, &ListenerSocketTCP::textOutput, CON, &Connection::dataInput );

		connect( LS, &ListenerSocketTCP::lineModeSupported, CON, &Connection::setLineModeSupport );

		connect( LS, &ListenerSocketTCP::terminalSizeChanged, CON, &Connection::setTerminalSize );

		connect( LS, &ListenerSocketTCP::disconnected, CM, &ConnectionManager::closeListener );

		LS->start();

		LS->setLineMode( CON->lineMode() );
	}
}
