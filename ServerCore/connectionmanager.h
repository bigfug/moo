#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <QObject>
#include <QMap>
#include <QMutex>

#include "mooglobal.h"

class Connection;
class ListenerSocket;

typedef QMap<ConnectionId, Connection *>	ConnectionNodeMap;

class ConnectionManager : public QObject
{
	Q_OBJECT

	explicit ConnectionManager( QObject *pParent = 0 );

	virtual ~ConnectionManager( void );

public:
	static ConnectionManager *instance( void );

	static void reset( void );

	ConnectionId fromPlayer( ObjectId pPlayerId );

	Connection *connection( ConnectionId pConnectionId );

	inline const ConnectionNodeMap &connectionList( void ) const
	{
		return( mConnectionNodeMap );
	}

	void closeListener( ListenerSocket *pLS );

	void processClosedSockets( void );

signals:
	
public slots:
	ConnectionId doConnect( ObjectId pListenerId );

	void doDisconnect( ConnectionId pConnectionId );

	void logon( ConnectionId pConnectionId, ObjectId pPlayerId );

	void logoff( ConnectionId pConnectionId );

private:
	static ConnectionManager	*mInstance;

	ConnectionNodeMap		 mConnectionNodeMap;
	ConnectionId			 mConnectionId;
	QMutex					 mClosedSocketMutex;
	QList<ListenerSocket *>	 mClosedSocketList;
};

#endif // CONNECTIONMANAGER_H
