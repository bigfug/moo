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

	ConnectionId connectionId( void ) const
	{
		return( mConnectionId );
	}

public slots:
	void setConnectionId( ConnectionId pConnectionId )
	{
		mConnectionId = pConnectionId;
	}

protected:
	ConnectionId			 mConnectionId;
};

#endif // LISTENERSOCKET_H
