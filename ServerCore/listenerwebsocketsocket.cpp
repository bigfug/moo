#include "listenerwebsocketsocket.h"

#include "objectmanager.h"
#include "connectionmanager.h"
#include "listenerwebsocket.h"

ListenerWebSocketSocket::ListenerWebSocketSocket( QObject *pParent, QWebSocket *pSocket )
	: ListenerSocket( pParent ), mSocket( pSocket )
{
	mConnectionId = ConnectionManager::instance()->doConnect( reinterpret_cast<ListenerWebSocket *>( parent() )->objectid() );

	Connection		*CON = ConnectionManager::instance()->connection( mConnectionId );

	connect( this, SIGNAL(textOutput(QString)), CON, SLOT(dataInput(QString)) );
	connect( CON, SIGNAL(textOutput(QString)), this, SLOT(textInput(QString)));

	connect( CON, SIGNAL(taskOutput(TaskEntry&)), ObjectManager::instance(), SLOT(doTask(TaskEntry&)));

	connect( mSocket, SIGNAL(disconnected()), this, SLOT(disconnected()) );

	//connect( mSocket, &QWebSocket::binaryFrameReceived, this, &ListenerWebSocketSocket::binaryFrameReceived );
	connect( mSocket, &QWebSocket::binaryMessageReceived, this, &ListenerWebSocketSocket::binaryMessageReceived );

	//connect( mSocket, &QWebSocket::textFrameReceived, this, &ListenerWebSocketSocket::textFrameReceived );
	connect( mSocket, &QWebSocket::textMessageReceived, this, &ListenerWebSocketSocket::textMessageReceived );

	connect( &mTimer, SIGNAL(timeout()), this, SLOT(inputTimeout()) );

	mTimer.singleShot( 1000, this, SLOT(inputTimeout()) );
}

void ListenerWebSocketSocket::disconnected()
{
	qInfo() << "Connection disconnected from" << mSocket->peerAddress();

	ConnectionManager::instance()->closeListener( this );
}

void ListenerWebSocketSocket::textInput( const QString &pText )
{
//	QByteArray		Buff = QByteArray( pText.toUtf8() );

//	if( mLineMode == Connection::EDIT )
//	{
//		Buff.append( "\r\n" );
//	}

	mSocket->sendTextMessage( pText );
}

void ListenerWebSocketSocket::inputTimeout()
{
	TaskEntry		 E( "", mConnectionId );

	ObjectManager::instance()->doTask( E );
}

void ListenerWebSocketSocket::binaryMessageReceived(const QByteArray &message)
{
	if( mTimer.isActive() )
	{
		mTimer.stop();
	}

	emit textOutput( message );
}

void ListenerWebSocketSocket::textMessageReceived(const QString &message)
{
	if( mTimer.isActive() )
	{
		mTimer.stop();
	}

	emit textOutput( message );
}
