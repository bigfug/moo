#ifndef LISTENERSOCKET_H
#define LISTENERSOCKET_H

#include "connection.h"

class ListenerSocket : public QObject
{
	Q_OBJECT

public:
	ListenerSocket( QObject *pParent = 0 )
		: QObject( pParent )
	{

	}

	virtual ~ListenerSocket( void ) {}

	inline ConnectionId connectionId( void ) const
	{
		return( mConnectionId );
	}

protected:
	ConnectionId			 mConnectionId;
};

#endif // LISTENERSOCKET_H
