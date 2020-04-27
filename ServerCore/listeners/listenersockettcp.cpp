#include "listenersockettcp.h"

#include <QHostAddress>

#include "../connectionmanager.h"

ListenerSocketTCP::ListenerSocketTCP( QObject *pParent, QTcpSocket *pSocket )
	: ListenerSocketTelnet( pParent ), mSocket( pSocket )
{
	qInfo() << "Connection established from" << mSocket->peerAddress();

	Connection		*CON = ConnectionManager::instance()->connection( mConnectionId );

	connect( mSocket, &QTcpSocket::disconnected, this, &ListenerSocketTCP::disconnected );

	connect( mSocket, &QTcpSocket::readyRead, this, &ListenerSocketTCP::readyRead );

	connect( CON, &Connection::connectionFlush, mSocket, &QTcpSocket::flush );

	start( this );
}

bool ListenerSocketTCP::isOpen() const
{
	return( mSocket->isOpen() );
}

qint64 ListenerSocketTCP::write(const QByteArray &A)
{
	return( mSocket->write( A ) );
}

qint64 ListenerSocketTCP::write(const char *p, qint64 l)
{
	return( mSocket->write( p, l ) );
}

void ListenerSocketTCP::disconnected( void )
{
	qInfo() << "Connection disconnected from" << mSocket->peerAddress();

	ConnectionManager::instance()->closeListener( this );
}

void ListenerSocketTCP::readyRead( void )
{
	read( mSocket->readAll() );
}
