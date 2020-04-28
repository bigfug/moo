#include "listenersocketwebsocket.h"

#include <QHostAddress>
#include <QDebug>

ListenerSocketWebSocket::ListenerSocketWebSocket( QObject *pParent, QWebSocket *pSocket )
	: ListenerSocketTelnet( pParent ), mSocket( pSocket )
{
	connect( mSocket, &QWebSocket::disconnected, this, &ListenerSocketWebSocket::socketDisconnected );

	connect( mSocket, &QWebSocket::binaryFrameReceived, this, &ListenerSocketWebSocket::binaryFrameReceived );

	connect( mSocket, &QWebSocket::textFrameReceived, this, &ListenerSocketWebSocket::textFrameReceived );
}

bool ListenerSocketWebSocket::isOpen() const
{
	return( mSocket->isValid() );
}

void ListenerSocketWebSocket::flush()
{
	mSocket->flush();
}

void ListenerSocketWebSocket::socketDisconnected()
{
	qInfo() << "Connection disconnected from" << mSocket->peerAddress();

	emit disconnected( this );
}

void ListenerSocketWebSocket::binaryFrameReceived( const QByteArray &message, bool isLastFrame )
{
	Q_UNUSED( isLastFrame )

	socketToTelnet( message );
}

void ListenerSocketWebSocket::textFrameReceived( const QString &message, bool isLastFrame )
{
	Q_UNUSED( isLastFrame )

	socketToTelnet( message.toLatin1() );
}

qint64 ListenerSocketWebSocket::writeToSocket( const QByteArray &A )
{
	return( mSocket->sendTextMessage( A ) );
}

qint64 ListenerSocketWebSocket::writeToSocket( const char *p, qint64 l )
{
	return( mSocket->sendTextMessage( QString::fromLatin1( p, l ) ) );
}
