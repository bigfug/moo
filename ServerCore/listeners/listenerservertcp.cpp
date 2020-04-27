#include "listenerservertcp.h"

#include "listenersockettelnet.h"

ListenerServerTCP::ListenerServerTCP( ObjectId pObjectId, quint16 pPort, QObject *pParent ) :
	ListenerServer( pObjectId, pParent )
{
	/*
	connect( &mServer, &ListenerServerTelnet::newConnection, this, &ListenerServerTCP::newConnection );

	mServer.listen( QHostAddress::Any, pPort );

	static const telnet_telopt_t my_telopts[] = {
		{ TELNET_TELOPT_ECHO,		TELNET_WILL, TELNET_DO },
		{ TELNET_TELOPT_TTYPE,		TELNET_WILL, TELNET_DO },
		{ TELNET_TELOPT_SGA,		TELNET_WILL, TELNET_DO },
//		{ TELNET_TELOPT_COMPRESS2, TELNET_WONT, TELNET_DO   },
//		{ TELNET_TELOPT_ZMP,       TELNET_WONT, TELNET_DO   },
//		{ TELNET_TELOPT_MSSP,      TELNET_WONT, TELNET_DO   },
//		{ TELNET_TELOPT_BINARY,    TELNET_WILL, TELNET_DO   },
		{ TELNET_TELOPT_NAWS,		TELNET_WILL, TELNET_DO },
		{ -1, 0, 0 }
	  };

	for( const telnet_telopt_t &ta : my_telopts )
	{
		mOptions.append( ta );
	}
	*/
}

void ListenerServerTCP::newConnection( void )
{
//	QTcpSocket		*S;

//	while( ( S = mServer.nextPendingConnection() ) != nullptr )
//	{
//		QSslSocket	*SS = qobject_cast<QSslSocket *>( S );

//		if( SS )
//		{
//			qInfo() << "Socket Encrypted:" << SS->isEncrypted();
//		}

//		ListenerSocketTelnet		*LS = new ListenerSocketTelnet( this, S );

//		if( !LS )
//		{
//			S->close();

//			continue;
//		}

//		LS->setOptions( mOptions );
//	}
}
