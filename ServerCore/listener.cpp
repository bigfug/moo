#include "listener.h"
#include "connectionmanager.h"
#include "objectmanager.h"
#include "taskentry.h"
#include <QCryptographicHash>
#include <QtEndian>
#include "mooexception.h"
#include <QMutexLocker>

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

#define TELNET_BINARY				(0)
#define TELNET_ECHO					(1)
#define TELNET_SUPPRESS_GO_AHEAD	(3)
#define TELNET_TERMINAL_TYPE		(24)
#define TELNET_NAWS					(31)

#define TELNET_MSSP					(70)
#define MSSP_VAR					(1)
#define MSSP_VAL					(2)

#define TELNET_MCCP					(86)

#define TELNET_MXP					(91)

#define TELNET_LINEMODE				(34)
#define LINEMODE_MODE				(1)
#define LINEMODE_MODE_EDIT			(1)
#define LINEMODE_MODE_TRAPSIG		(2)
#define LINEMODE_MODE_ACK			(4)
#define LINEMODE_MODE_SOFTTAB		(8)
#define LINEMODE_MODE_LITECHO		(16)
#define LINEMODE_FORWARDMASK		(2)
#define LINEMODE_SLC				(3)

#define SLC_NOSUPPORT				(0)
#define SLC_CANTCHANGE				(1)
#define SLC_VALUE					(2)
#define SLC_DEFAULT					(3)

#define SLC_LEVELBITS				(3)

Listener::Listener( ObjectId pObjectId, quint16 pPort, QObject *pParent ) :
	QObject( pParent ), mObjectId( pObjectId ), mServer( this )
{
	connect( &mServer, SIGNAL( newConnection() ), this, SLOT( newConnection() ) );

	mServer.listen( QHostAddress::Any, pPort );

	mOptions.append( TelnetOption( TELNET_ECHO,					TELNET_WONT, TELNET_DO ) );
	mOptions.append( TelnetOption( TELNET_SUPPRESS_GO_AHEAD,	TELNET_WONT, TELNET_DONT ) );
	mOptions.append( TelnetOption( TELNET_TERMINAL_TYPE,		TELNET_WONT, TELNET_DO ) );
	mOptions.append( TelnetOption( TELNET_NAWS,					TELNET_WONT, TELNET_DO ) );
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
}

bool ListenerSocket::option( quint8 pOption ) const
{
	for( const TelnetOption &TE : mOptions )
	{
		if( TE.mOption == pOption )
		{
			return( TE.mLocal == TELNET_WILL );
		}
	}

	return( false );
}

bool ListenerSocket::echo() const
{
	return( false ); //option( TELNET_ECHO ) );
}

void ListenerSocket::sendData( const QByteArray &pData )
{
//	qDebug() << "ListenerSocket::sendData" << pData;

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

	mSocket->write( pData );
}

void ListenerSocket::processInput( const QByteArray &pData )
{
//	qDebug() << "processInput" << pData;

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
		else if( !mTelnetSequence.isEmpty() )
		{
			mTelnetSequence.push_back( ch );

			// check for IAC IAC escape

			if( mTelnetSequence.size() == 2 && ch == TELNET_IAC )
			{
				mBuffer.push_back( ch );

				mTelnetSequence.clear();
			}
			else if( mTelnetSequence.size() == 3 && quint8( mTelnetSequence.at( 1 ) ) != TELNET_SB )
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

				if( echo() )
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

			if( echo() )
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

void ListenerSocket::processTelnetSequence( const QByteArray &pData )
{
//	qDebug() << "ListenerSocket::processTelnetSequence" << pData;

	const quint8		Command = pData.at( 1 );
	const quint8		Option  = pData.at( 2 );

#if 0
	QString		CmdStr;

	switch( Command )
	{
		case TELNET_DO:		CmdStr = "DO";		break;
		case TELNET_DONT:	CmdStr = "DONT";	break;
		case TELNET_WILL:	CmdStr = "WILL";	break;
		case TELNET_WONT:	CmdStr = "WONT";	break;
		case TELNET_SB:		CmdStr = "SB";		break;
		default:
			CmdStr = QString::number( Command );
	}

	QString		OptStr;

	switch( Option )
	{
		case TELNET_ECHO:				OptStr = "ECHO";				break;
		case TELNET_SUPPRESS_GO_AHEAD:	OptStr = "SUPPRESS_GO_AHEAD";	break;
		case TELNET_LINEMODE:			OptStr = "LINEMODE";			break;
		case TELNET_TERMINAL_TYPE:		OptStr = "TERMINAL_TYPE";		break;
		case TELNET_NAWS:				OptStr = "TELNET_NAWS";			break;
		default:
			OptStr = QString::number( Option );
	}

	qDebug() << CmdStr << OptStr;
#endif

	if( Command == TELNET_DO )
	{
		for( TelnetOption &TE : mOptions )
		{
			if( TE.mOption == Option && TE.mLocal != TELNET_WILL )
			{
				QByteArray	Msg;

				appendTelnetSequence( Msg, TELNET_WILL, Option );

				sendData( Msg );

				TE.mLocal = TELNET_WILL;

				break;
			}
		}
	}

	if( Command == TELNET_DONT )
	{
		for( TelnetOption &TE : mOptions )
		{
			if( TE.mOption == Option && TE.mLocal != TELNET_WONT )
			{
				QByteArray	Msg;

				appendTelnetSequence( Msg, TELNET_WONT, Option );

				sendData( Msg );

				TE.mLocal = TELNET_WONT;

				break;
			}
		}
	}

	if( Command == TELNET_WILL || Command == TELNET_WONT )
	{
		for( TelnetOption &TE : mOptions )
		{
			if( TE.mOption == Option )
			{
				TE.mRemote = Command;

				break;
			}
		}
	}

	if( Option == TELNET_TERMINAL_TYPE )
	{
		if( Command == TELNET_WILL )
		{
			QByteArray	Msg;

			appendTelnetSequence( Msg, TELNET_SB, TELNET_TERMINAL_TYPE );

			Msg.append( 1 );

			Msg.append( TELNET_IAC );
			Msg.append( TELNET_SE );

			sendData( Msg );
		}
		else if( Command == TELNET_SB )
		{
			const quint8		IS = pData.at( 3 );

			if( IS == 0 )
			{
				QByteArray	TermName = pData.mid( 4 );

				TermName.truncate( TermName.size() - 2 );

				qDebug() << "TERMINAL_TYPE" << QString( TermName );
			}
		}
	}

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

void ListenerSocket::inputTimeout( void )
{
	QByteArray	Msg;

	for( TelnetOption TE : mOptions )
	{
		appendTelnetSequence( Msg, TE.mRemote, TE.mOption );
	}

//	qDebug() << "inputTimeout" << Msg;

	if( !Msg.isEmpty() )
	{
		sendData( Msg );
	}

	TaskEntry		 E( "", mConnectionId );

	ObjectManager::instance()->queueTask( E );
}

void ListenerSocket::setLineMode( Connection::LineMode pLineMode )
{
	QByteArray	Msg;

	if( pLineMode == Connection::EDIT )
	{
		setTelnetOption( TELNET_ECHO, TELNET_WONT );
		setTelnetOption( TELNET_SUPPRESS_GO_AHEAD, TELNET_WONT );

		appendTelnetSequence( Msg, TELNET_WONT, TELNET_ECHO );
		appendTelnetSequence( Msg, TELNET_WONT, TELNET_SUPPRESS_GO_AHEAD );
	}
	else
	{
		setTelnetOption( TELNET_ECHO, TELNET_WILL );
		setTelnetOption( TELNET_SUPPRESS_GO_AHEAD, TELNET_WILL );

		appendTelnetSequence( Msg, TELNET_WILL, TELNET_ECHO );
		appendTelnetSequence( Msg, TELNET_WILL, TELNET_SUPPRESS_GO_AHEAD );
	}

	if( !Msg.isEmpty() )
	{
		sendData( Msg );
	}

//	qDebug() << "setLineMode" << Msg;

	mLineMode = pLineMode;
}

void ListenerSocket::setTelnetOption(quint8 pOption, quint8 pCommand)
{
	for( TelnetOption &TE : mOptions )
	{
		if( TE.mOption == pOption )
		{
			TE.mLocal = pCommand;

			return;
		}
	}
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
		processInput( mSocket->readAll() );
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

void ListenerSocket::appendTelnetSequence( QByteArray &pA, const quint8 p1, const quint8 p2)
{
	pA.append( TELNET_IAC );
	pA.append( p1 );
	pA.append( p2 );
}
