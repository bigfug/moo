#include "listenersockettcp.h"

#include <QHostAddress>
#include <QDebug>

ListenerSocketTCP::ListenerSocketTCP( QObject *pParent, QTcpSocket *pSocket )
	: ListenerSocketTelnet( pParent ), mSocket( pSocket )
{
	qInfo() << "Connection established from" << mSocket->peerAddress();

	connect( mSocket, &QTcpSocket::disconnected, this, &ListenerSocketTCP::socketDisconnected );

	connect( mSocket, &QTcpSocket::readyRead, this, &ListenerSocketTCP::readyRead );
}

bool ListenerSocketTCP::isOpen() const
{
	return( mSocket->isOpen() );
}

void ListenerSocketTCP::flush()
{
	mSocket->flush();
}

qint64 ListenerSocketTCP::writeToSocket(const QByteArray &A)
{
	return( mSocket->write( A ) );
}

qint64 ListenerSocketTCP::writeToSocket(const char *p, qint64 l)
{
	return( mSocket->write( p, l ) );
}

void ListenerSocketTCP::socketDisconnected( void )
{
	qInfo() << "Connection disconnected from" << mSocket->peerAddress();

	emit disconnected( this );
}

void ListenerSocketTCP::readyRead( void )
{
	socketToTelnet( mSocket->readAll() );
}
