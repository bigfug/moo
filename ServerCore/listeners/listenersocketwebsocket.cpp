#include "listenersocketwebsocket.h"

#include "objectmanager.h"
#include "connectionmanager.h"
#include "listenerserverwebsocket.h"

ListenerSocketWebSocket::ListenerSocketWebSocket( QObject *pParent, QWebSocket *pSocket )
	: ListenerSocketTelnet( pParent ), mSocket( pSocket )
{
	connect( mSocket, &QWebSocket::disconnected, this, &ListenerSocketWebSocket::disconnected );

	connect( mSocket, &QWebSocket::binaryMessageReceived, this, &ListenerSocketWebSocket::binaryMessageReceived );

	connect( mSocket, &QWebSocket::textFrameReceived, this, &ListenerSocketWebSocket::textFrameReceived );

	start( this );
}

bool ListenerSocketWebSocket::isOpen() const
{
	return( mSocket->isValid() );
}

void ListenerSocketWebSocket::disconnected()
{
	qInfo() << "Connection disconnected from" << mSocket->peerAddress();

	ConnectionManager::instance()->closeListener( this );
}

void ListenerSocketWebSocket::binaryMessageReceived( const QByteArray &message )
{
	read( message );
}

void ListenerSocketWebSocket::textFrameReceived( const QString &message, bool isLastFrame )
{
	Q_UNUSED( isLastFrame )

	read( message.toLatin1() );
}

qint64 ListenerSocketWebSocket::write( const QByteArray &A )
{
	return( mSocket->sendTextMessage( A ) );
}

qint64 ListenerSocketWebSocket::write( const char *p, qint64 l )
{
	return( mSocket->sendTextMessage( QString::fromLatin1( p, l ) ) );
}
