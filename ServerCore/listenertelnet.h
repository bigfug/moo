#ifndef LISTENERTELNET_H
#define LISTENERTELNET_H

#include "mooglobal.h"
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include "connection.h"
#include <QTimer>
#include <QTimerEvent>
#include <QMutex>

#include "libtelnet.h"

#include "listenerserver.h"

class ListenerTelnet : public ListenerServer
{
	Q_OBJECT

public:
	explicit ListenerTelnet( ObjectId pObjectId, quint16 pPort, QObject *pParent = 0 );

	virtual ~ListenerTelnet( void ) {}

private slots:
	void newConnection( void );

private:
	QTcpServer					 mServer;
	ObjectId					 mListeningObjectId;
	QVector<telnet_telopt_t>	 mOptions;
};

#endif // LISTENERTELNET_H
