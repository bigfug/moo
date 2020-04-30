#ifndef LISTENERSERVERSERIAL_H
#define LISTENERSERVERSERIAL_H

#include "listenerserver.h"

#include <QSerialPort>
#include <QPointer>

#include "listenersocket.h"

class ListenerServerSerial : public ListenerServer
{
	Q_OBJECT

public:
	explicit ListenerServerSerial( ObjectId pObjectId, QString pPort, int pBaud, QObject *pParent = 0 );

	virtual ~ListenerServerSerial( void ) {}

private slots:
	void serialReadReady( void );

private:
	QSerialPort					*mSerialPort;
	ObjectId					 mListeningObjectId;
	QPointer<ListenerSocket>	 mSocket;
};

#endif // LISTENERSERVERSERIAL_H
