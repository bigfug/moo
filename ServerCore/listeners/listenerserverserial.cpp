#include "listenerserverserial.h"

#include <QDebug>

#include "listenerserialmodem.h"
#include "connectionmanager.h"
#include "objectmanager.h"

ListenerServerSerial::ListenerServerSerial( ObjectId pObjectId, QString pPort, int pBaud, QObject *pParent )
	: ListenerServer( pObjectId, pParent )
{
	mSerialPort = new QSerialPort( pPort, this );

	connect( mSerialPort, &QSerialPort::readyRead, this, &ListenerServerSerial::serialReadReady );

	mSerialPort->setBaudRate( pBaud );

	if( !mSerialPort->open( QSerialPort::ReadWrite ) )
	{
		qWarning() << "Can't open Serial Port" << pPort;
	}
}

void ListenerServerSerial::serialReadReady( void )
{
	ListenerSerialModem		*Modem = qobject_cast<ListenerSerialModem *>( mSocket );

	if( !Modem )
	{
		Modem->processSerialRead();
	}
	else
	{
		QByteArray		LineData;

		while( !( LineData = mSerialPort->readLine() ).isEmpty() )
		{
			LineData = LineData.trimmed();

			if( !LineData.compare( "RING" ) )
			{
				mSerialPort->write( "ATA\r\n" );
			}
			else if( !LineData.startsWith( "CONNECT " ) )
			{
				ListenerSerialModem		*LS = new ListenerSerialModem( mSerialPort, this );

				if( LS )
				{
					mSocket = qobject_cast<ListenerSocket *>( LS );

					ConnectionManager	*CM = ConnectionManager::instance();

					ConnectionId		 CID = CM->doConnect( objectid() );
					Connection			*CON = CM->connection( CID );

					LS->setConnectionId( CID );

					connect( CON, &Connection::listenerOutput, LS, &ListenerSerialModem::connectionToSerial );
					connect( CON, &Connection::connectionClosed, LS, &ListenerSerialModem::close );
					connect( CON, &Connection::connectionFlush, LS, &ListenerSerialModem::flush );

					connect( CON, &Connection::taskOutput, ObjectManager::instance(), &ObjectManager::doTask );

					connect( LS, &ListenerSerialModem::serialToConnection, CON, &Connection::listenerInput );
					connect( LS, &ListenerSerialModem::disconnected, CM, &ConnectionManager::closeListener );

					connect( LS, &ListenerSerialModem::ready, [=]( void )
					{
						CON->performTask( "" );
					} );

					LS->start();
				}
			}
		}
	}
}

