#include "listener.h"
#include "connectionmanager.h"
#include "objectmanager.h"
#include "taskentry.h"
#include <QCryptographicHash>
#include <QtEndian>
#include "mooexception.h"
#include <QMutexLocker>

Listener::Listener( ObjectId pObjectId, quint16 pPort, QObject *pParent ) :
	QObject( pParent ), mObjectId( pObjectId ), mServer( this )
{
	connect( &mServer, SIGNAL( newConnection() ), this, SLOT( newConnection() ) );

	mServer.listen( QHostAddress::Any, pPort );

	static const telnet_telopt_t my_telopts[] = {
		{ TELNET_TELOPT_ECHO,		TELNET_WONT, TELNET_DO },
		{ TELNET_TELOPT_TTYPE,		TELNET_WILL, TELNET_DO },
		{ TELNET_TELOPT_SGA,		TELNET_WONT, TELNET_DONT },
//		{ TELNET_TELOPT_COMPRESS2, TELNET_WONT, TELNET_DO   },
//		{ TELNET_TELOPT_ZMP,       TELNET_WONT, TELNET_DO   },
//		{ TELNET_TELOPT_MSSP,      TELNET_WONT, TELNET_DO   },
//		{ TELNET_TELOPT_BINARY,    TELNET_WILL, TELNET_DO   },
		{ TELNET_TELOPT_NAWS,		TELNET_WILL, TELNET_DO },
		{ -1, 0, 0 }
	  };

	for( const telnet_telopt_t &ta : my_telopts )
	{
		mOptions.append( ta );
	}
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

		if( !LS )
		{
			S->close();
		}

		LS->setOptions( mOptions );
	}
}

ListenerSocket::ListenerSocket( QObject *pParent, QTcpSocket *pSocket ) :
	QObject( pParent ), mSocket( pSocket ), mLineMode( Connection::EDIT )
{
	mDataReceived    = false;
	mWebSocketHeader = false;
	mWebSocketActive = false;
	mLastChar  = 0;
	mAnsiEsc = 0;
	mAnsiPos = 0;

	qDebug() << "Connection established from" << mSocket->peerAddress();

	mConnectionId = ConnectionManager::instance()->doConnect( reinterpret_cast<Listener *>( parent() )->objectid() );

	Connection		*CON = ConnectionManager::instance()->connection( mConnectionId );

	connect( this, SIGNAL(textOutput(QString)), CON, SLOT(dataInput(QString)) );
	connect( CON, SIGNAL(textOutput(QString)), this, SLOT(textInput(QString)));

	connect( CON, SIGNAL(taskOutput(TaskEntry&)), ObjectManager::instance(), SLOT(doTask(TaskEntry&)));

	connect( CON, SIGNAL(lineMode(Connection::LineMode)), this, SLOT(setLineMode(Connection::LineMode)) );
	connect( this, SIGNAL(lineModeSupported(bool)), CON, SLOT(setLineModeSupport(bool)) );

	connect( mSocket, SIGNAL(disconnected()), this, SLOT(disconnected()) );
	connect( mSocket, SIGNAL(readyRead()), this, SLOT(readyRead()) );

	connect( &mTimer, SIGNAL(timeout()), this, SLOT(inputTimeout()) );

	mTimer.singleShot( 1000, this, SLOT(inputTimeout()) );

	mTelnet = telnet_init( mOptions.constData(), &ListenerSocket::telnetEventHandlerStatic, 0, this );

	telnet_negotiate( mTelnet, TELNET_DO, TELNET_TELOPT_NAWS );
	telnet_negotiate( mTelnet, TELNET_DO, TELNET_TELOPT_TTYPE );
}

bool ListenerSocket::echo() const
{
	return( false ); //option( TELNET_ECHO ) );
}

void ListenerSocket::sendData( const QByteArray &pData )
{
#if defined( DEBUG_LISTERNER )
	qDebug() << "ListenerSocket::sendData" << pData;
#endif

	if( mWebSocketActive )
	{
		QByteArray	Pkt;
		int			S = pData.size();

		Pkt.append( quint8( 0x80 + 0x01 ) );	// FIN + TEXT

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

		Pkt.append( pData );

		mSocket->write( Pkt );

		return;
	}

	//mSocket->write( pData );

	telnet_send( mTelnet, pData.constData(), pData.size() );
}

void ListenerSocket::processInput( const QByteArray &pData )
{
#if defined( DEBUG_LISTERNER )
	qDebug() << "processInput" << pData;
#endif

	for( int i = 0 ; i < pData.size() ; i++ )
	{
		quint8		ch = pData.at( i );

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
//		else if( !mTelnetSequence.isEmpty() )
//		{
//			mTelnetSequence.push_back( ch );

//			// check for IAC IAC escape

//			if( mTelnetSequence.size() == 2 && ch == TELNET_IAC )
//			{
//				mBuffer.push_back( ch );

//				mTelnetSequence.clear();
//			}
//			else if( mTelnetSequence.size() == 3 && quint8( mTelnetSequence.at( 1 ) ) != TELNET_SB )
//			{
//				processTelnetSequence( mTelnetSequence );

//				mTelnetSequence.clear();
//			}
//			else if( ch == TELNET_SE && mLastChar == TELNET_IAC )
//			{
//				processTelnetSequence( mTelnetSequence );

//				mTelnetSequence.clear();
//			}
//		}
//		else if( ch == TELNET_IAC )
//		{
//			mTelnetSequence.push_back( ch );
//		}
		else if( mLineMode == Connection::REALTIME )
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
					//mLocalEcho = true;

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

					if( echo() )
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
						//mLocalEcho = false;
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

				if( echo() )
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

						if( echo() )
						{
							QByteArray	Tmp;

							Tmp.append( "\x1b[D" );
							Tmp.append( mBuffer.mid( mAnsiPos ) );
							Tmp.append( QString( " \x1b[%1D" ).arg( mBuffer.size() + 1 - mAnsiPos ) );

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

				if( echo() )
				{
					QByteArray	Tmp;

					Tmp.append( mBuffer.mid( mAnsiPos ) );
					Tmp.append( QString( " \x1b[%1D" ).arg( mBuffer.size() + 1 - mAnsiPos ) );

					sendData( Tmp );
				}
			}
		}
		else
		{
			mBuffer.insert( mAnsiPos++, ch );

			if( echo() )
			{
				QByteArray	Tmp;

				if( mAnsiPos < mBuffer.size() )
				{
					Tmp.append( mBuffer.mid( mAnsiPos - 1 ).append( QString( "\x1b[%1D" ).arg( mBuffer.size() - mAnsiPos ) ) );
				}
				else
				{
					Tmp.append( ch );
				}

				//sendData( Tmp );
			}
		}

		mLastChar = ch;
	}

	// echo the data back to the client

	if( echo() )
	{
		sendData( pData );
	}
}

/*
void ListenerSocket::processTelnetSequence( const QByteArray &pData )
{

	if( Option == TELNET_NAWS )
	{
		if( Command == TELNET_SB )
		{
			quint16	w = ( quint8( pData.at( 3 ) ) << 8 ) | quint8( pData.at( 4 ) );
			quint16	h = ( quint8( pData.at( 5 ) ) << 8 ) | quint8( pData.at( 6 ) );

			qDebug() << "TERMINAL_NAWS" << w << h;

			Connection		*CON = ConnectionManager::instance()->connection( mConnectionId );

			if( CON )
			{
				CON->setTerminalSize( QSize( w, h ) );
			}
		}
	}
}
*/

void ListenerSocket::inputTimeout( void )
{
	TaskEntry		 E( "", mConnectionId );

	ObjectManager::instance()->queueTask( E );
}

void ListenerSocket::setLineMode( Connection::LineMode pLineMode )
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

void ListenerSocket::disconnected( void )
{
	qInfo() << "Connection disconnected from" << mSocket->peerAddress();

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
		const QByteArray	SckDat = mSocket->readAll();

		qDebug() << "RECV" << SckDat;

		telnet_recv( mTelnet, SckDat.constData(), SckDat.size() );

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
	QByteArray		Buff = QByteArray( pText.toUtf8() );

	if( mLineMode == Connection::EDIT )
	{
		Buff.append( "\r\n" );
	}

	sendData( Buff );
}

void ListenerSocket::processAnsiSequence( const QByteArray &pData )
{
	if( pData.size() == 3 )
	{
		switch( static_cast<quint8>( pData.at( 2 ) ) )
		{
			case 'C':	// CURSOR FORWARD
				if( mAnsiPos < mBuffer.size() )
				{
					mAnsiPos++;

					sendData( "\x1b[C" );
				}
				break;

			case 'D':	// CURSOR BACK
				if( mAnsiPos > 0 )
				{
					mAnsiPos--;

					sendData( "\x1b[D" );
				}
				break;

		}
	}
}

void ListenerSocket::appendTelnetSequence( QByteArray &pA, const quint8 p1, const quint8 p2)
{
	pA.append( TELNET_IAC );
	pA.append( p1 );
	pA.append( p2 );
}

void ListenerSocket::telnetEventHandlerStatic(telnet_t *telnet, telnet_event_t *event, void *user_data)
{
	Q_UNUSED( telnet )

	((ListenerSocket *)user_data)->telnetEventHandler( event );
}

void ListenerSocket::telnetEventHandler(telnet_event_t *event)
{
	if( event->type == TELNET_EV_DATA )
	{
		qDebug() << "DATA" << QByteArray::fromRawData( event->data.buffer, event->data.size );

		processInput( QByteArray::fromRawData( event->data.buffer, event->data.size ) );
	}
	else if( event->type == TELNET_EV_SEND )
	{
		qDebug() << "SEND" << QByteArray::fromRawData( event->data.buffer, event->data.size );

		mSocket->write( event->data.buffer, event->data.size );
	}
	else if( event->type == TELNET_EV_WILL )
	{
		switch( event->neg.telopt )
		{
			case TELNET_TELOPT_TTYPE:
				telnet_subnegotiation( mTelnet, TELNET_TELOPT_TTYPE, "\1", 1 );
				break;
		}
	}
	else if( event->type == TELNET_EV_SUBNEGOTIATION )
	{
		switch( event->sub.telopt )
		{
			case TELNET_TELOPT_TTYPE:
				qDebug() << "TTYPE" << QByteArray::fromRawData( event->sub.buffer, event->sub.size );
				break;

			case TELNET_TELOPT_NAWS:
				{
					quint16	w = ( event->sub.buffer[ 0 ] << 8 ) | event->sub.buffer[ 1 ];
					quint16	h = ( event->sub.buffer[ 2 ] << 8 ) | event->sub.buffer[ 3 ];

					qDebug() << "TERMINAL_NAWS" << w << h;

					Connection		*CON = ConnectionManager::instance()->connection( mConnectionId );

					if( CON )
					{
						CON->setTerminalSize( QSize( w, h ) );
					}
				}
				break;
		}
	}
	else
	{
		qDebug() << event->type;
	}
}
