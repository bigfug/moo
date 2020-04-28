#include "listenersockettelnet.h"

#include <QCryptographicHash>
#include <QtEndian>
#include <QJsonDocument>
#include <QDebug>

#include "connectionmanager.h"
#include "listenerserver.h"
#include "objectmanager.h"

//#define DEBUG_LISTENER

#define TELNET_TELOPT_GMCP (0xc9)

ListenerSocketTelnet::ListenerSocketTelnet( QObject *pParent ) :
	ListenerSocket( pParent ), mCursorPosition( 0 ), mTelnet( nullptr ), mLineMode( Connection::NOT_SET )
{
	mLastChar  = 0;
	mLocalEcho = false;

	//------------------------------------------------------------------------
	// Telnet Options

	static const telnet_telopt_t my_telopts[] = {
		{ TELNET_TELOPT_ECHO,		TELNET_WILL, TELNET_DO },
		{ TELNET_TELOPT_TTYPE,		TELNET_WILL, TELNET_DO },
		{ TELNET_TELOPT_SGA,		TELNET_WILL, TELNET_DO },
		{ TELNET_TELOPT_NAWS,		TELNET_WILL, TELNET_DO },
		{ -1, 0, 0 }
	  };

	for( const telnet_telopt_t &ta : my_telopts )
	{
		mOptions.append( ta );
	}
}

ListenerSocketTelnet::~ListenerSocketTelnet()
{
	if( mTelnet )
	{
		telnet_free( mTelnet );

		mTelnet = Q_NULLPTR;
	}
}

void ListenerSocketTelnet::start( void )// ListenerSocketTelnet *pThis )
{
	mTelnet = telnet_init( mOptions.constData(), &ListenerSocketTelnet::telnetEventHandlerStatic, 0, this );

	telnet_negotiate( mTelnet, TELNET_DO,   TELNET_TELOPT_NAWS );		// ask for client window size
	telnet_negotiate( mTelnet, TELNET_DO,   TELNET_TELOPT_TTYPE );		// ask for terminal type
	telnet_negotiate( mTelnet, TELNET_WILL, TELNET_TELOPT_GMCP );		// Generic Mud Communication Protocol

	//------------------------------------------------------------------------
	// Initial timer

	connect( &mTimer, &QTimer::timeout, this, &ListenerSocketTelnet::inputTimeout );

	mTimer.setSingleShot( true );

	mTimer.start( 500 );
}

bool ListenerSocketTelnet::echo() const
{
	return( mLocalEcho );
}

void ListenerSocketTelnet::sendData( const QByteArray &pData )
{
#if defined( DEBUG_LISTENER )
	qDebug() << "ListenerTelnetSocket::sendData" << pData;
#endif

	telnet_send_text( mTelnet, pData.constData(), pData.size() );
}

void ListenerSocketTelnet::processInput( const QByteArray &pData )
{
#if defined( DEBUG_LISTENER )
	qDebug() << "processInput" << pData;
#endif

	Connection		*CON = ConnectionManager::instance()->connection( mConnectionId );

	for( int i = 0 ; i < pData.size() ; i++ )
	{
		quint8		ch = pData.at( i );

		if( mLineMode == Connection::REALTIME )
		{
			if( CON )
			{
				CON->processInput( QChar( ch ) );
			}
		}
		else if( ch == '\r' || ch == '\n' )
		{
			if( mBuffer.isEmpty() )
			{
				if( ch == '\n' && mLastChar == '\r' )
				{

				}
				else
				{
					emit textOutput( mBuffer );
				}
			}
			else
			{
				emit textOutput( mBuffer );

				mBuffer.clear();

				mCursorPosition = 0;
			}
		}
		else
		{
			if( ch == '\b' )
			{
				if( mCursorPosition > 0 )
				{
					mBuffer.remove( mCursorPosition - 1, 1 );

					mCursorPosition--;
				}
			}
			else
			{
				if( ch >= 0x20 && ch < 0x7f )
				{
					mBuffer.insert( mCursorPosition, ch );

					mCursorPosition++;
				}
			}
		}

		mLastChar = ch;
	}
}

void ListenerSocketTelnet::inputTimeout( void )
{
	TaskEntry		 E( "", mConnectionId );

	ObjectManager::instance()->doTask( E );
}

void ListenerSocketTelnet::setLineMode( Connection::LineMode pLineMode )
{
	if( pLineMode == mLineMode )
	{
		return;
	}

	if( pLineMode == Connection::EDIT )
	{
		telnet_negotiate( mTelnet, TELNET_WONT, TELNET_TELOPT_ECHO );
		telnet_negotiate( mTelnet, TELNET_WONT, TELNET_TELOPT_SGA );
	}
	else
	{
		telnet_negotiate( mTelnet, TELNET_WILL, TELNET_TELOPT_ECHO );
		telnet_negotiate( mTelnet, TELNET_WILL, TELNET_TELOPT_SGA );
	}

	mLineMode = pLineMode;
}

void ListenerSocketTelnet::sendGMCP( const QByteArray &pGMCP )
{
	telnet_subnegotiation( mTelnet, TELNET_TELOPT_GMCP, pGMCP.constData(), pGMCP.size() );
}

void ListenerSocketTelnet::read( QByteArray A )
{
	if( mTelnet )
	{
		telnet_recv( mTelnet, A.constData(), A.size() );
	}
	else
	{
		processInput( A );
	}
}

void ListenerSocketTelnet::stopTimer()
{
	if( mTimer.isActive() )
	{
		mTimer.stop();
	}
}

void ListenerSocketTelnet::textInput( const QString &pText )
{
	QByteArray		Buff = QByteArray( pText.toUtf8() );

	if( mLineMode == Connection::EDIT )
	{
		Buff.append( "\r\n" );
	}

	sendData( Buff );
}

void ListenerSocketTelnet::telnetEventHandlerStatic(telnet_t *telnet, telnet_event_t *event, void *user_data)
{
	Q_UNUSED( telnet )

	((ListenerSocketTelnet *)user_data)->telnetEventHandler( event );
}

void ListenerSocketTelnet::telnetEventHandler( telnet_event_t *event )
{
	switch( event->type )
	{
		case TELNET_EV_DATA:
			processInput( QByteArray::fromRawData( event->data.buffer, event->data.size ) );
			break;

		case TELNET_EV_SEND:
			{
				if( !isOpen() )
				{
					qWarning() << QString::fromLatin1( event->data.buffer, event->data.size );
				}
				else
				{
					write( event->data.buffer, event->data.size );
				}
			}
			break;

		case TELNET_EV_WILL:
			{
#if defined( DEBUG_LISTENER )
				qDebug() << "WILL" << event->neg.telopt;
#endif

				switch( event->neg.telopt )
				{
					case TELNET_TELOPT_ECHO:
						mLocalEcho = false;
						break;

					case TELNET_TELOPT_TTYPE:
						telnet_subnegotiation( mTelnet, TELNET_TELOPT_TTYPE, "\1", 1 );
						break;
				}
			}
			break;

		case TELNET_EV_WONT:
			{
#if defined( DEBUG_LISTENER )
				qDebug() << "WONT" << event->neg.telopt;
#endif

				switch( event->neg.telopt )
				{
					case TELNET_TELOPT_ECHO:
						mLocalEcho = true;
						break;
				}
			}
			break;

		case TELNET_EV_DO:
#if defined( DEBUG_LISTENER )
			qDebug() << "DO" << event->neg.telopt;
#endif
			switch( event->sub.telopt )
			{
				case TELNET_TELOPT_GMCP:
#if defined( DEBUG_LISTENER )
					qDebug() << "GMCP enabled";
#endif
					break;
			}
			break;

		case TELNET_EV_DONT:
#if defined( DEBUG_LISTENER )
			qDebug() << "DONT" << event->neg.telopt;
#endif
			switch( event->sub.telopt )
			{
				case TELNET_TELOPT_GMCP:
					qDebug() << "GMCP disabled";
					break;
			}
			break;

		case TELNET_EV_SUBNEGOTIATION:
			{
				switch( event->sub.telopt )
				{
					case TELNET_TELOPT_TTYPE:
#if defined( DEBUG_LISTENER )
						qDebug() << "TTYPE" << QByteArray::fromRawData( event->sub.buffer, event->sub.size );
#endif
						break;

					case TELNET_TELOPT_NAWS:
						{
							quint16	w = ( event->sub.buffer[ 0 ] << 8 ) | event->sub.buffer[ 1 ];
							quint16	h = ( event->sub.buffer[ 2 ] << 8 ) | event->sub.buffer[ 3 ];

#if defined( DEBUG_LISTENER )
							qDebug() << "TERMINAL_NAWS" << w << h;
#endif

							emit terminalSizeChanged( QSize( w, h ) );
						}
						break;

					case TELNET_TELOPT_GMCP:
						{
							QByteArray		A( event->sub.buffer, event->sub.size );
							QString			P;
							QJsonDocument	JSON;

							while( !A.isEmpty() && A[ 0 ] != ' ' )
							{
								P.append( QChar( A[ 0 ] ) );

								A.remove( 0, 1 );
							}

							if( !A.isEmpty() )
							{
								A.remove( 0, 1 );

								JSON = QJsonDocument::fromJson( A );

							}

#if defined( DEBUG_LISTENER )
							qDebug() << P << A << JSON.toJson();
#endif
						}
						break;

					default:
#if defined( DEBUG_LISTENER )
						qDebug() << "Unhandled Telnet subnegotiation:" << event->sub.telopt;
#endif
						break;
				}
			}
			break;

		case TELNET_EV_TTYPE:
#if defined( DEBUG_LISTENER )
			qDebug() << "Telnet Terminal Type:" << event->ttype.cmd << event->ttype.name;
#endif
			break;

		default:
#if defined( DEBUG_LISTENER )
			qDebug() << "Unhandled Telnet event type:" << event->type;
#endif
			break;
	}
}
