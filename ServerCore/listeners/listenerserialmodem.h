#ifndef LISTENERSERIALMODEM_H
#define LISTENERSERIALMODEM_H

#include <QObject>
#include <QSerialPort>

#include "listenersocket.h"

class ListenerSerialModem : public ListenerSocket
{
	Q_OBJECT

public:
	ListenerSerialModem( QSerialPort *pSerialPort, QObject *pParent = Q_NULLPTR );

public:
	void processSerialRead( void );

public slots:
	void start( void )
	{
		emit ready();
	}

	void connectionToSerial( const QString &S );

	void close( void );

	void flush( void )
	{
		mSerialPort->flush();
	}

signals:
	void disconnected( ListenerSocket *pSocket );

	void ready( void );

	void serialToConnection( const QString &S );

private:
	QSerialPort			*mSerialPort;
};

#endif // LISTENERSERIALMODEM_H
