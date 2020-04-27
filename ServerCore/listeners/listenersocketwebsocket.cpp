#include "listenersocketwebsocket.h"

#include "objectmanager.h"
#include "connectionmanager.h"
#include "listenerserverwebsocket.h"

ListenerSocketWebSocket::ListenerSocketWebSocket( QObject *pParent, QWebSocket *pSocket )
	: ListenerSocket( pParent ), mSocket( pSocket )
{
	mConnectionId = ConnectionManager::instance()->doConnect( reinterpret_cast<ListenerServerWebSocket *>( parent() )->objectid() );

	Connection		*CON = ConnectionManager::instance()->connection( mConnectionId );

	connect( this, SIGNAL(textOutput(QString)), CON, SLOT(dataInput(QString)) );
	connect( CON, SIGNAL(textOutput(QString)), this, SLOT(textInput(QString)));

	connect( CON, SIGNAL(taskOutput(TaskEntry&)), ObjectManager::instance(), SLOT(doTask(TaskEntry&)));

	connect( mSocket, SIGNAL(disconnected()), this, SLOT(disconnected()) );

	//connect( mSocket, &QWebSocket::binaryFrameReceived, this, &ListenerWebSocketSocket::binaryFrameReceived );
	connect( mSocket, &QWebSocket::binaryMessageReceived, this, &ListenerSocketWebSocket::binaryMessageReceived );

	//connect( mSocket, &QWebSocket::textFrameReceived, this, &ListenerWebSocketSocket::textFrameReceived );
	connect( mSocket, &QWebSocket::textMessageReceived, this, &ListenerSocketWebSocket::textMessageReceived );

	connect( &mTimer, SIGNAL(timeout()), this, SLOT(inputTimeout()) );

	mTimer.singleShot( 1000, this, SLOT(inputTimeout()) );
}

void ListenerSocketWebSocket::disconnected()
{
	qInfo() << "Connection disconnected from" << mSocket->peerAddress();

	ConnectionManager::instance()->closeListener( this );
}

void ListenerSocketWebSocket::textInput( const QString &pText )
{
//	QByteArray		Buff = QByteArray( pText.toUtf8() );

//	if( mLineMode == Connection::EDIT )
//	{
//		Buff.append( "\r\n" );
//	}

	mSocket->sendTextMessage( pText );
}

void ListenerSocketWebSocket::inputTimeout()
{
	TaskEntry		 E( "", mConnectionId );

	ObjectManager::instance()->doTask( E );
}

void ListenerSocketWebSocket::binaryMessageReceived(const QByteArray &message)
{
	if( mTimer.isActive() )
	{
		mTimer.stop();
	}

	emit textOutput( message );
}

void ListenerSocketWebSocket::textMessageReceived(const QString &message)
{
	if( mTimer.isActive() )
	{
		mTimer.stop();
	}

	emit textOutput( message );
}
