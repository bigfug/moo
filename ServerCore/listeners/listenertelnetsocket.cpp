#include "listenertelnetsocket.h"

#include <QCryptographicHash>
#include <QtEndian>
#include <QJsonDocument>

#include "connectionmanager.h"
#include "listenerserver.h"
#include "objectmanager.h"

//#define DEBUG_LISTENER

#define TELNET_TELOPT_GMCP (0xc9)

ListenerTelnetSocket::ListenerTelnetSocket( QObject *pParent, QTcpSocket *pSocket ) :
	ListenerSocket( pParent ), mSocket( pSocket ), mCursorPosition( 0 ), mTelnet( nullptr ), mLineMode( Connection::EDIT )
{
	mDataReceived    = false;
	mWebSocketHeader = false;
	mWebSocketActive = false;

	mLastChar  = 0;
	mLocalEcho = false;

	qInfo() << "Connection established from" << mSocket->peerAddress();

	mConnectionId = ConnectionManager::instance()->doConnect( reinterpret_cast<ListenerServer *>( parent() )->objectid() );

	Connection		*CON = ConnectionManager::instance()->connection( mConnectionId );

	connect( this, &ListenerTelnetSocket::textOutput, CON, &Connection::dataInput );
	connect( CON, &Connection::textOutput, this, &ListenerTelnetSocket::textInput );

	connect( CON, &Connection::taskOutput, ObjectManager::instance(), &ObjectManager::doTask );

	connect( CON, &Connection::lineModeChanged, this, &ListenerTelnetSocket::setLineMode );
	connect( this, &ListenerTelnetSocket::lineModeSupported, CON, &Connection::setLineModeSupport );

	connect( mSocket, &QTcpSocket::disconnected, this, &ListenerTelnetSocket::disconnected );
	connect( mSocket, &QTcpSocket::readyRead, this, &ListenerTelnetSocket::readyRead );

	connect( CON, &Connection::gmcpOutput, this, &ListenerTelnetSocket::sendGMCP );

	connect( CON, &Connection::connectionClosed, this, &ListenerTelnetSocket::close );
	connect( CON, &Connection::connectionFlush, mSocket, &QTcpSocket::flush );

	connect( &mTimer, &QTimer::timeout, this, &ListenerTelnetSocket::inputTimeout );

	mTimer.singleShot( 500, this, SLOT(inputTimeout()) );

	mTelnet = telnet_init( mOptions.constData(), &ListenerTelnetSocket::telnetEventHandlerStatic, 0, this );
}

bool ListenerTelnetSocket::echo() const
{
	return( mLocalEcho );
}

void ListenerTelnetSocket::sendData( const QByteArray &pData )
{
#if defined( DEBUG_LISTENER )
	qDebug() << "ListenerTelnetSocket::sendData" << pData;
#endif

	telnet_send( mTelnet, pData.constData(), pData.size() );
}

void ListenerTelnetSocket::processInput( const QByteArray &pData )
{
#if defined( DEBUG_LISTENER )
	qDebug() << "processInput" << pData;
#endif

	for( int i = 0 ; i < pData.size() ; i++ )
	{
		quint8		ch = pData.at( i );

		if( !mDataReceived && ch == 0x00 && mBuffer.compare( "<policy-file-request/>" ) == 0 )
		{
			QString        Policy =
				"<?xml version=\"1.0\"?>\r\n"
				"<!DOCTYPE cross-domain-policy SYSTEM \"/xml/dtds/cross-domain-policy.dtd\">\r\n"
				"<cross-domain-policy>\r\n"
				"\t<site-control permitted-cross-domain-policies=\"master-only\" />\r\n"
				"\t<allow-access-from domain=\"www.bigfug.com\" to-ports=\"1123\" />\r\n"
				"</cross-domain-policy>\r\n";

			mSocket->write( Policy.toUtf8() );

			mSocket->close();

			return;
		}

		if( mLineMode == Connection::REALTIME )
		{
			Connection		*CON = ConnectionManager::instance()->connection( mConnectionId );

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
				else if( mWebSocketHeader )
				{
					QStringList        Reply;

					Reply << QStringLiteral( "HTTP/1.1 101 Web Socket Protocol Handshake" );
					Reply << QStringLiteral( "Upgrade: websocket" );
					Reply << QStringLiteral( "Connection: Upgrade" );
					Reply << QString( "Sec-WebSocket-Accept: %1" ).arg( mWebSocketAccept );
					Reply << QStringLiteral( "Server: ArtMOO" );

					if( !mWebSocketOrigin.isEmpty() )
					{
						Reply << QString( "Access-Control-Allow-Origin: %1" ).arg( mWebSocketOrigin );
					}

					if( !mWebSocketProtocol.isEmpty() )
					{
						Reply << QString( "Sec-WebSocket-Protocol: %1" ).arg( mWebSocketProtocol );
					}

					Reply << QStringLiteral( "\r\n" );

#if defined( DEBUG_LISTENER )
					qDebug() << Reply.join( "\r\n" );
#endif

					mSocket->write( Reply.join( "\r\n" ).toLatin1() );

					mWebSocketHeader = false;
					mWebSocketActive = true;
					mLocalEcho = true;

//					if( pData.size() > i + 1 )
//					{
//						QByteArray        Sub = pData.mid( i + 1 );

//						if( !Sub.isEmpty() )
//						{
//							processInput( Sub );
//						}
//					}

					telnet_negotiate( mTelnet, TELNET_DO,   TELNET_TELOPT_BINARY );

					telnet_negotiate( mTelnet, TELNET_DO,   TELNET_TELOPT_NAWS );
					telnet_negotiate( mTelnet, TELNET_DO,   TELNET_TELOPT_TTYPE );

					telnet_negotiate( mTelnet, TELNET_DO,   TELNET_TELOPT_ECHO );
					telnet_negotiate( mTelnet, TELNET_WONT, TELNET_TELOPT_ECHO );

					telnet_negotiate( mTelnet, TELNET_DONT, TELNET_TELOPT_SGA );
//					telnet_negotiate( mTelnet, TELNET_DONT, TELNET_TELOPT_SGA );

					telnet_negotiate( mTelnet, TELNET_DO,   TELNET_TELOPT_LINEMODE );

					telnet_negotiate( mTelnet, TELNET_WILL, TELNET_TELOPT_GMCP );

//					setLineMode( Connection::EDIT );

//					TaskEntry		 E( "", mConnectionId );

//					ObjectManager::instance()->doTask( E );

					return;
				}
				else
				{
					emit textOutput( mBuffer );
				}
			}
			else
			{
				if( mWebSocketHeader )
				{
					QStringList              Parts = mBuffer.split( ':' );
					const QString            K = Parts.takeFirst();
					const QString            V = Parts.join( ":" ).trimmed();

					if( K.compare( "Sec-WebSocket-Key" ) == 0 )
					{
						static const QString    S1 = QStringLiteral( "258EAFA5-E914-47DA-95CA-C5AB0DC85B11" );
						const QString           S2 = V + S1;
						const QByteArray        S3 = QCryptographicHash::hash( S2.toUtf8(), QCryptographicHash::Sha1 );
						const QString           S4 = S3.toBase64();

						mWebSocketAccept = S4;
					}

					if( K.compare( "Origin" ) == 0 )
					{
						mWebSocketOrigin = V;
					}

					if( K.compare( "Sec-WebSocket-Protocol" ) == 0 )
					{
						QStringList		Protocols = V.split( ", " );

						if( Protocols.contains( "chat" ) )
						{
							mWebSocketProtocol = "chat";
						}
						else if( Protocols.contains( "binary" ) )
						{
							mWebSocketProtocol = "binary";
						}
						else if( Protocols.contains( "plain" ) )
						{
							mWebSocketProtocol = "plain";
						}
					}
				}
				else if( !mDataReceived )
				{
					if( mBuffer.startsWith( "GET /" ) && mBuffer.endsWith( " HTTP/1.1" ) )
					{
						mWebSocketHeader = true;
						mLocalEcho = false;
						mWebSocketProtocol = "chat";
					}
					else
					{
						emit textOutput( mBuffer );
					}

					mDataReceived = true;
				}
				else
				{
					emit textOutput( mBuffer );
				}

				mBuffer.clear();

				mCursorPosition = 0;
			}
		}
		else
		{
			//qDebug() << QString::number( ch, 16 );

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

void ListenerTelnetSocket::inputTimeout( void )
{
	if( !mWebSocketActive )
	{
		telnet_negotiate( mTelnet, TELNET_DO, TELNET_TELOPT_NAWS );
		telnet_negotiate( mTelnet, TELNET_DO, TELNET_TELOPT_TTYPE );

		telnet_negotiate( mTelnet, TELNET_DO,   TELNET_TELOPT_BINARY );

//		telnet_negotiate( mTelnet, TELNET_DO,   TELNET_TELOPT_ECHO );
//		telnet_negotiate( mTelnet, TELNET_WONT, TELNET_TELOPT_ECHO );

//		telnet_negotiate( mTelnet, TELNET_DONT, TELNET_TELOPT_SGA );

//		telnet_negotiate( mTelnet, TELNET_DO,   TELNET_TELOPT_LINEMODE );

		telnet_negotiate( mTelnet, TELNET_WILL, TELNET_TELOPT_GMCP );

		setLineMode( Connection::EDIT );
	}

	TaskEntry		 E( "", mConnectionId );

	ObjectManager::instance()->doTask( E );
}

void ListenerTelnetSocket::setLineMode( Connection::LineMode pLineMode )
{
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

void ListenerTelnetSocket::sendGMCP( const QByteArray &pGMCP )
{
	telnet_subnegotiation( mTelnet, TELNET_TELOPT_GMCP, pGMCP.constData(), pGMCP.size() );
}

void ListenerTelnetSocket::close()
{
	mSocket->close();
}

void ListenerTelnetSocket::disconnected( void )
{
	qInfo() << "Connection disconnected from" << mSocket->peerAddress();

	ConnectionManager::instance()->closeListener( this );
}

void ListenerTelnetSocket::readyRead( void )
{
	if( mTimer.isActive() )
	{
		mTimer.stop();
	}

	if( !mWebSocketActive )
	{
		const QByteArray	SckDat = mSocket->readAll();

		//qDebug() << "RECV" << SckDat;

		if( mTelnet )
		{
			telnet_recv( mTelnet, SckDat.constData(), SckDat.size() );
		}
		else
		{
			processInput( SckDat );
		}

		return;
	}

	mWebSocketBuffer.append( mSocket->readAll() );

	while( mWebSocketBuffer.size() > int( sizeof( quint16 ) ) )
	{
		quint8				H1 = mWebSocketBuffer.at( 0 );
		quint8				H2 = mWebSocketBuffer.at( 1 );
		size_t				U  = 2;
		quint64			L;
		QByteArray			M;

		// process the length

		quint8				PAYLOAD = H2 & 0x7f;

		if( PAYLOAD <= 125 )
		{
			L = PAYLOAD;
		}
		else if( PAYLOAD == 126 )
		{
			if( mWebSocketBuffer.size() < int( U + sizeof( quint16 ) ) )
			{
				return;
			}

			quint16	S16;

			memcpy( &S16, mWebSocketBuffer.constData() + U, sizeof( quint16 ) );

			L = qFromBigEndian<quint16>( S16 );

			U += sizeof( quint16 );
		}
		else if( PAYLOAD == 127 )
		{
			if( mWebSocketBuffer.size() < int( U + sizeof( quint64 ) ) )
			{
				return;
			}

			quint64	S64;

			memcpy( &S64, mWebSocketBuffer.constData() + U, sizeof( quint64 ) );

			L = qFromBigEndian<quint64>( S64 );

			U += sizeof( quint64 );
		}

		// Extract the mask

		const bool		MASK = ( H2 & 0x80 ) != 0;

		if( MASK )
		{
			if( mWebSocketBuffer.size() < int( U + 4 ) )
			{
				return;
			}

			for( int i = 0 ; i < 4 ; i++ )
			{
				M.append( mWebSocketBuffer.at( U + i ) );
			}

			U += 4;
		}

		// check we have a full frame

		if( mWebSocketBuffer.size() < int( U + L ) )
		{
			return;
		}

		const quint8	OP = ( H1 & 0x0f );

		switch( OP )
		{
			case 0x00:		// Continuation Frame
				break;

			case 0x01:		// Text
			case 0x02:		// Binary
				if( MASK )
				{
					QByteArray	D = mWebSocketBuffer.mid( U, L );

					for( int i = 0 ; i < D.size() ; i++ )
					{
						D[ i ] = D[ i ] ^ M[ i % 4 ];
					}

					if( mTelnet )
					{
						telnet_recv( mTelnet, D.constData(), D.size() );
					}
					else
					{
						mSocket->write( D );
					}
				}
				else
				{
					if( mTelnet )
					{
						const QByteArray	D = mWebSocketBuffer.mid( U, L );

						telnet_recv( mTelnet, D.constData(), D.size() );
					}
					else
					{
						mSocket->write( mWebSocketBuffer.mid( U, L ) );
					}
				}
				break;

			case 0x08:		// Connection Close
				break;

			case 0x09:		// Ping
				break;

			case 0x0a:		// Pong
				break;

			default:		// Reserved
				break;
		}

		mWebSocketBuffer.remove( 0, U + L );
	}
}

void ListenerTelnetSocket::textInput( const QString &pText )
{
	QByteArray		Buff = QByteArray( pText.toUtf8() );

	if( mLineMode == Connection::EDIT )
	{
		Buff.append( "\r\n" );
	}

	sendData( Buff );
}

void ListenerTelnetSocket::telnetEventHandlerStatic(telnet_t *telnet, telnet_event_t *event, void *user_data)
{
	Q_UNUSED( telnet )

	((ListenerTelnetSocket *)user_data)->telnetEventHandler( event );
}

void ListenerTelnetSocket::telnetEventHandler(telnet_event_t *event)
{
	switch( event->type )
	{
		case TELNET_EV_DATA:
			processInput( QByteArray::fromRawData( event->data.buffer, event->data.size ) );
			break;

		case TELNET_EV_SEND:
			{
				if( !mSocket->isOpen() )
				{
					qWarning() << QString::fromLatin1( event->data.buffer, event->data.size );
				}
				else if( mWebSocketActive )
				{
					QByteArray     Pkt;
					int            S = event->data.size;

					if( mWebSocketProtocol == "binary" )
					{
						Pkt.append( quint8( 0x80 + 0x02 ) );    // FIN + BINARY
					}
					else
					{
						Pkt.append( quint8( 0x80 + 0x01 ) );    // FIN + TEXT
					}

					if( S <= 125 )
					{
						Pkt.append( quint8( S ) );
					}
					else if( S <= 0xffff )
					{
						Pkt.append( quint8( 126 ) );
						Pkt.append( quint8( ( S >> 8 ) & 0xff ) );
						Pkt.append( quint8( ( S >> 0 ) & 0xff ) );
					}

					Pkt.append( QByteArray::fromRawData( event->data.buffer, event->data.size ) );

					mSocket->write( Pkt );
				}
				else
				{
					mSocket->write( event->data.buffer, event->data.size );
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
					qDebug() << "GMCP enabled";
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
						//qDebug() << "TTYPE" << QByteArray::fromRawData( event->sub.buffer, event->sub.size );
						break;

					case TELNET_TELOPT_NAWS:
						{
							quint16	w = ( event->sub.buffer[ 0 ] << 8 ) | event->sub.buffer[ 1 ];
							quint16	h = ( event->sub.buffer[ 2 ] << 8 ) | event->sub.buffer[ 3 ];

#if defined( DEBUG_LISTENER )
							qDebug() << "TERMINAL_NAWS" << w << h;
#endif

							Connection		*CON = ConnectionManager::instance()->connection( mConnectionId );

							if( CON )
							{
								CON->setTerminalSize( QSize( w, h ) );
							}
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

							qDebug() << P << A << JSON.toJson();
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
