#ifndef LISTENERWEBSOCKET_H
#define LISTENERWEBSOCKET_H

#include "mooglobal.h"
#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include "connection.h"
#include <QTimer>
#include <QTimerEvent>
#include <QMutex>

#include "libtelnet.h"

#include "listenerserver.h"
#include "listenersocket.h"

class ListenerServerWebSocket : public ListenerServer
{
	Q_OBJECT

public:
	explicit ListenerServerWebSocket( ObjectId pObjectId, quint16 pPort, QObject *pParent = 0 );

	virtual ~ListenerServerWebSocket( void ) {}

private slots:
	void newConnection( void );

private:
	QWebSocketServer			 mServer;
	ObjectId					 mListeningObjectId;
};

#endif // LISTENERWEBSOCKET_H
