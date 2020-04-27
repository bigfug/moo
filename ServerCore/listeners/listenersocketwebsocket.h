#ifndef LISTENERWEBSOCKETSOCKET_H
#define LISTENERWEBSOCKETSOCKET_H

#include <QWebSocket>
#include <QTimer>

#include "listenersockettelnet.h"

class ListenerSocketWebSocket : public ListenerSocketTelnet
{
	Q_OBJECT

public:
	ListenerSocketWebSocket( QObject *pParent, QWebSocket *pSocket );

	virtual ~ListenerSocketWebSocket( void ) {}

	virtual bool isOpen( void ) const Q_DECL_OVERRIDE;

private slots:
	void disconnected( void );

	void binaryMessageReceived(const QByteArray &message);

	void textFrameReceived( const QString &message, bool isLastFrame );

protected slots:
	virtual void close( void ) Q_DECL_OVERRIDE
	{
		mSocket->close();
	}

	virtual qint64 write( const QByteArray &A ) Q_DECL_OVERRIDE;
	virtual qint64 write( const char *p, qint64 l ) Q_DECL_OVERRIDE;

private:
	QWebSocket					*mSocket;
};

#endif // LISTENERWEBSOCKETSOCKET_H
