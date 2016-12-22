#include "listenertelnet.h"

#include "listenertelnetsocket.h"

ListenerTelnet::ListenerTelnet( ObjectId pObjectId, quint16 pPort, QObject *pParent ) :
	ListenerServer( pObjectId, pParent ), mServer( this )
{
	connect( &mServer, SIGNAL( newConnection() ), this, SLOT( newConnection() ) );

	mServer.listen( QHostAddress::Any, pPort );

	static const telnet_telopt_t my_telopts[] = {
		{ TELNET_TELOPT_ECHO,		TELNET_WONT, TELNET_DO },
		{ TELNET_TELOPT_TTYPE,		TELNET_WILL, TELNET_DO },
		{ TELNET_TELOPT_SGA,		TELNET_WONT, TELNET_DONT },
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
}

void ListenerTelnet::newConnection( void )
{
	QTcpSocket		*S;

	while( ( S = mServer.nextPendingConnection() ) != 0 )
	{
		ListenerTelnetSocket		*LS = new ListenerTelnetSocket( this, S );

		if( !LS )
		{
			S->close();
		}

		LS->setOptions( mOptions );
	}
}
