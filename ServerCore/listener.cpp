#include "listener.h"
#include "connectionmanager.h"
#include "objectmanager.h"
#include "taskentry.h"
#include <QCryptographicHash>
#include <QtEndian>
#include "mooexception.h"
#include <QMutexLocker>

#define TELNET_ECHO					(1)

#define TELNET_SE					(240)	// End of subnegotiation parameters
#define TELNET_NOP					(241)	// No operation.
#define TELNET_DATA_MARK			(242)	// The data stream portion of a Synch. This should always be accompanied by a TCP Urgent notification.
#define TELNET_BREAK				(243)	// NVT character BRK.
#define TELNET_INTERRUPT_PROCESS	(244)	// The function IP
#define TELNET_ABORT_OUTPUT			(245)	// The function AO
#define TELNET_ARE_YOU_THERE		(246)	// The function AYT
#define TELNET_ERASE_CHARACTER		(247)	// The function EC
#define TELNET_ERASE_LINE			(248)	// The function EL.
#define TELNET_GO_AHREAD			(249)	// The GA signal.
#define TELNET_SB					(250)	// Indicates that what follows is subnegotiation of the indicated option.
#define TELNET_WILL					(251)	// Indicates the desire to begin performing, or confirmation that you are now performing, the indicated option.
#define TELNET_WONT					(252)	// Indicates the refusal to perform, or continue performing, the indicated option.
#define TELNET_DO					(253)	// Indicates the request that the other party perform, or confirmation that you are expecting the other party to perform, the indicated option.
#define TELNET_DONT					(254)	// Indicates the demand that the other party stop performing, or confirmation that you are no longer expecting the other party to perform, the indicated option.
#define TELNET_IAC					(255)	// Data Byte 255.

#define TELNET_MSSP					(70)
#define MSSP_VAR					(1)
#define MSSP_VAL					(2)

#define TELNET_MCCP					(86)

#define TELNET_MXP					(91)

Listener::Listener( ObjectId pObjectId, quint16 pPort, QObject *pParent ) :
	QObject( pParent ), mObjectId( pObjectId ), mServer( this )
{
	connect( &mServer, SIGNAL( newConnection() ), this, SLOT( newConnection() ) );

	mServer.listen( QHostAddress::Any, pPort );
}

Listener::~Listener()
{
}

void Listener::newConnection( void )
{
	QTcpSocket		*S;

	while( ( S = mServer.nextPendingConnection() ) != 0 )
	{
		ListenerSocket		*LS = new ListenerSocket( this, S );

		if( LS == 0 )
		{
			S->close();
		}
	}
}

ListenerSocket::ListenerSocket( QObject *pParent, QTcpSocket *pSocket ) :
	QObject( pParent ), mSocket( pSocket )
{
	mDataReceived    = false;
	mWebSocketHeader = false;
	mWebSocketActive = false;
	mLocalEcho = false;
	mLastChar  = 0;
	mTelnetDepth = 0;
	mAnsiEsc = 0;
	mAnsiPos = 0;

	qDebug() << "Connection established from" << mSocket->peerAddress();

	mConnectionId = ConnectionManager::instance()->doConnect( reinterpret_cast<Listener *>( parent() )->objectid() );

	Connection		*CON = ConnectionManager::instance()->connection( mConnectionId );

	connect( this, SIGNAL(textOutput(QString)), CON, SLOT(dataInput(QString)) );
	connect( CON, SIGNAL(textOutput(QString)), this, SLOT(textInput(QString)));

	connect( CON, SIGNAL(taskOutput(TaskEntry&)), ObjectManager::instance(), SLOT(doTask(TaskEntry&)));

	connect( mSocket, SIGNAL(disconnected()), this, SLOT(disconnected()) );
	connect( mSocket, SIGNAL(readyRead()), this, SLOT(readyRead()) );

	connect( &mTimer, SIGNAL(timeout()), this, SLOT(inputTimeout()) );

	mTimer.singleShot( 1000, this, SLOT(inputTimeout()) );
}

void ListenerSocket::sendData( const QByteArray &pData )
{
	if( mWebSocketActive )
	{
		QByteArray	Pkt;
		int			S = pData.size();

		Pkt.append( uint8_t( 0x80 + 0x01 ) );	// FIN + TEXT

		if( S <= 125 )
		{
			Pkt.append( uint8_t( S ) );
		}
		else if( S <= 0xffff )
		{
			Pkt.append( uint8_t( 126 ) );
			Pkt.append( uint8_t( ( S >> 8 ) & 0xff ) );
			Pkt.append( uint8_t( ( S >> 0 ) & 0xff ) );
		}

		Pkt.append( pData );

		mSocket->write( Pkt );

		return;
	}

	mSocket->write( pData );
}

void ListenerSocket::processInput( const QByteArray &pData )
{
	for( int i = 0 ; i < pData.size() ; i++ )
	{
		uint8_t		ch = pData.at( i );

		if( ch == 0x00 && !mDataReceived && mBuffer.compare( "<policy-file-request/>" ) == 0 )
		{
			QString		Policy;

			Policy += "<?xml version=\"1.0\"?>\r\n";
			Policy += "<!DOCTYPE cross-domain-policy SYSTEM \"/xml/dtds/cross-domain-policy.dtd\">\r\n";
			Policy += "<cross-domain-policy>\r\n";
			Policy += "\t<site-control permitted-cross-domain-policies=\"master-only\" />\r\n";
			Policy += "\t<allow-access-from domain=\"www.bigfug.com\" to-ports=\"1123\" />\r\n";
			Policy += "</cross-domain-policy>\r\n";

			mSocket->write( Policy.toUtf8() );

			mSocket->close();

			return;
		}
		else if( !mTelnetSequence.isEmpty() )
		{
			mTelnetSequence.push_back( ch );

			// check for IAC IAC escape

			if( mTelnetSequence.size() == 2 && ch == TELNET_IAC )
			{
				mBuffer.push_back( ch );

				mTelnetSequence.clear();
			}
			else if( mTelnetSequence.size() == 3 && uint8_t( mTelnetSequence.at( 1 ) ) != TELNET_SB )
			{
				processTelnetSequence( mTelnetSequence );

				mTelnetSequence.clear();
			}
			else if( ch == TELNET_SE && mLastChar == TELNET_IAC )
			{
				processTelnetSequence( mTelnetSequence );

				mTelnetSequence.clear();
			}
		}
		else if( ch == TELNET_IAC )
		{
			mTelnetSequence.push_back( ch );
		}
		else if( ch == '\r' || ch == '\n' )
		{
			if( mBuffer.isEmpty() )
			{
				if( ch == '\n' && mLastChar == '\r' )
				{
					// Do nothing for \r\n pair
				}
				else if( mWebSocketHeader )
				{
					QString		Reply;

					Reply += "HTTP/1.1 101 Switching Protocols\r\n";
					Reply += "Upgrade: websocket\r\n";
					Reply += "Connection: Upgrade\r\n";
					Reply += QString( "Sec-WebSocket-Accept: %1\r\n" ).arg( mWebSocketAccept ).toUtf8();
					Reply += "Sec-WebSocket-Protocol: chat\r\n";
					Reply += "\r\n";

					mSocket->write( Reply.toUtf8() );

					mWebSocketHeader = false;
					mWebSocketActive = true;
					mLocalEcho = true;

					if( pData.size() > i + 1 )
					{
						QByteArray		Sub = pData.mid( i + 1 );

						if( !Sub.isEmpty() )
						{
							processInput( Sub );
						}
					}

					return;
				}
				else
				{
					// preserve empty lines

					if( mLocalEcho )
					{
						sendData( "\r\n" );
					}

					emit textOutput( mBuffer );
				}
			}
			else
			{
				//qDebug() << mBuffer;

				if( mWebSocketHeader )
				{
					QStringList				Parts = mBuffer.split( ':' );
					const QString			K = Parts.takeFirst();
					const QString			V = Parts.join( ":" ).trimmed();

					if( K.compare( "Sec-WebSocket-Key" ) == 0 )
					{
						static const QString	S1 = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
						const QString			S2 = V + S1;
						const QByteArray		S3 = QCryptographicHash::hash( S2.toUtf8(), QCryptographicHash::Sha1 );
						const QString			S4 = S3.toBase64();

						mWebSocketAccept = S4;
					}
				}
				else if( !mDataReceived )
				{
					if( mBuffer.compare( "GET / HTTP/1.1" ) == 0 )
					{
						mWebSocketHeader = true;
						mLocalEcho = false;
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

				if( mLocalEcho )
				{
					sendData( "\r\n" );
				}

				mAnsiPos = 0;

				mBuffer.clear();
			}
		}
		else if( mAnsiEsc == 1 )
		{
			if( ch == '[' )
			{
				mAnsiEsc++;

				mAnsiSeq.append( ch );
			}
			else
			{
				mBuffer.append( 0x1B );
				mBuffer.append( ch );

				mAnsiEsc = 0;
			}
		}
		else if( mAnsiEsc == 2 )
		{
			mAnsiSeq.append( ch );

			if( ch >= 64 && ch <= 126 )
			{
				processAnsiSequence( mAnsiSeq );

				mAnsiEsc = 0;
			}
		}
		else if( ch < 0x20 && ch != 0x09 )
		{
			switch( ch )
			{
				case 0x08:	// BACKSPACE
					if( mAnsiPos > 0 )
					{
						mBuffer.remove( --mAnsiPos, 1 );

						if( mLocalEcho )
						{
							QByteArray	Tmp;

							Tmp.append( "\e[D" );
							Tmp.append( mBuffer.mid( mAnsiPos ) );
							Tmp.append( QString( " \e[%1D" ).arg( mBuffer.size() + 1 - mAnsiPos ) );

							sendData( Tmp );
						}
					}
					break;

				case 0x09:
					break;

				case 0x0e:	// SHIFT OUT
				case 0x0f:	// SHIFT IN
					break;

				case 0x1b:	// ESCAPE
					mAnsiSeq.clear();
					mAnsiSeq.append( ch );
					mAnsiEsc++;
					break;

			}
		}
		else if( ch == 0x7f )	// DELETE
		{
			if( mAnsiPos < mBuffer.size() )
			{
				mBuffer.remove( mAnsiPos, 1 );

				if( mLocalEcho )
				{
					QByteArray	Tmp;

					Tmp.append( mBuffer.mid( mAnsiPos ) );
					Tmp.append( QString( " \e[%1D" ).arg( mBuffer.size() + 1 - mAnsiPos ) );

					sendData( Tmp );
				}
			}
		}
		else
		{
			mBuffer.insert( mAnsiPos++, ch );

			if( mLocalEcho )
			{
				QByteArray	Tmp;

				if( mAnsiPos < mBuffer.size() )
				{
					Tmp.append( mBuffer.mid( mAnsiPos - 1 ).append( QString( "\e[%1D" ).arg( mBuffer.size() - mAnsiPos ) ) );
				}
				else
				{
					Tmp.append( ch );
				}

				sendData( Tmp );
			}
		}

		mLastChar = ch;
	}

	// echo the data back to the client

	if( mLocalEcho )
	{
		//sendData( pData );
	}
}

void ListenerSocket::processTelnetSequence( const QByteArray &pData )
{
	if( static_cast<uint8_t>( pData.at( 1 ) ) == TELNET_DO )
	{
		switch( static_cast<uint8_t>( pData.at( 2 ) ) )
		{
			case TELNET_ECHO:
				mLocalEcho = true;
				break;

			case TELNET_MSSP:
				{
					QByteArray	Msg;

					Msg.append( TELNET_IAC );
					Msg.append( TELNET_SB );
					Msg.append( TELNET_MSSP );

					Msg.append( MSSP_VAR );
					Msg.append( "NAME" );
					Msg.append( MSSP_VAL );
					Msg.append( "ArtMOO" );

					Msg.append( MSSP_VAR );
					Msg.append( "PLAYERS" );
					Msg.append( MSSP_VAL );
					Msg.append( "1" );

					Msg.append( MSSP_VAR );
					Msg.append( "UPTIME" );
					Msg.append( MSSP_VAL );
					Msg.append( "1" );

					Msg.append( TELNET_IAC );
					Msg.append( TELNET_SE );

					mSocket->write( Msg );
				}
				break;
		}
	}
}

void ListenerSocket::inputTimeout( void )
{
	QByteArray	Msg;

	Msg.append( TELNET_IAC );
	Msg.append( TELNET_WILL );
	Msg.append( TELNET_ECHO );

//	Msg.append( TELNET_IAC );
//	Msg.append( TELNET_WILL );
//	Msg.append( TELNET_MSSP );

	mSocket->write( Msg );

	mLocalEcho = true;

	//Connection		*CON = ConnectionManager::instance()->connection( mConnectionId );

	TaskEntry		 E( "", mConnectionId );

	ObjectManager::instance()->queueTask( E );
}

void ListenerSocket::disconnected( void )
{
	qDebug() << "Connection disconnected from" << mSocket->peerAddress();

	ConnectionManager::instance()->closeListener( this );
}

void ListenerSocket::readyRead( void )
{
	if( mTimer.isActive() )
	{
		mTimer.stop();
	}

	if( !mWebSocketActive )
	{
		processInput( mSocket->readAll() );
	}

	mWebSocketBuffer.append( mSocket->readAll() );

	while( mWebSocketBuffer.size() > int( sizeof( uint16_t ) ) )
	{
		uint8_t				H1 = mWebSocketBuffer.at( 0 );
		uint8_t				H2 = mWebSocketBuffer.at( 1 );
		size_t				U  = 2;
		uint64_t			L;
		QByteArray			M;

		// process the length

		uint8_t				PAYLOAD = H2 & 0x7f;

		if( PAYLOAD <= 125 )
		{
			L = PAYLOAD;
		}
		else if( PAYLOAD == 126 )
		{
			if( mWebSocketBuffer.size() < int( U + sizeof( uint16_t ) ) )
			{
				return;
			}

			uint16_t	S16;

			memcpy( &S16, mWebSocketBuffer.constData() + U, sizeof( uint16_t ) );

			L = qFromBigEndian<uint16_t>( S16 );

			U += sizeof( uint16_t );
		}
		else if( PAYLOAD == 127 )
		{
			if( mWebSocketBuffer.size() < int( U + sizeof( uint64_t ) ) )
			{
				return;
			}

			uint64_t	S64;

			memcpy( &S64, mWebSocketBuffer.constData() + U, sizeof( uint64_t ) );

			L = qFromBigEndian<uint64_t>( S64 );

			U += sizeof( uint64_t );
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

		const uint8_t	OP = ( H1 & 0x0f );

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

					processInput( D );
				}
				else
				{
					processInput( mWebSocketBuffer.mid( U, L ) );
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

void ListenerSocket::textInput( const QString &pText )
{
	QByteArray		Buff = QByteArray( pText.toUtf8() ).append( "\r\n" );

	sendData( Buff );
}

void ListenerSocket::processAnsiSequence( const QByteArray &pData )
{
	if( pData.size() == 3 )
	{
		switch( static_cast<uint8_t>( pData.at( 2 ) ) )
		{
			case 'C':	// CURSOR FORWARD
				if( mAnsiPos < mBuffer.size() )
				{
					mAnsiPos++;

					sendData( "\e[C" );
				}
				break;

			case 'D':	// CURSOR BACK
				if( mAnsiPos > 0 )
				{
					mAnsiPos--;

					sendData( "\e[D" );
				}
				break;

		}
	}
}

