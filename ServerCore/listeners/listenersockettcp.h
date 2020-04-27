#ifndef LISTENERSOCKETTCP_H
#define LISTENERSOCKETTCP_H

#include <QObject>
#include <QTcpSocket>

#include "listenersockettelnet.h"

class ListenerSocketTCP : public ListenerSocketTelnet
{
	Q_OBJECT

public:
	ListenerSocketTCP( QObject *pParent, QTcpSocket *pSocket );

	virtual ~ListenerSocketTCP( void ) {}

	virtual bool isOpen( void ) const Q_DECL_OVERRIDE;

protected slots:
	virtual void close( void ) Q_DECL_OVERRIDE
	{
		mSocket->close();
	}

	virtual qint64 write( const QByteArray &A ) Q_DECL_OVERRIDE;
	virtual qint64 write( const char *p, qint64 l ) Q_DECL_OVERRIDE;

	void disconnected( void );
	void readyRead( void );

private:
	QTcpSocket					*mSocket;
};

#endif // LISTENERSOCKETTCP_H
