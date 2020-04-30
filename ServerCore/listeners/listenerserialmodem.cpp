#include "listenerserialmodem.h"

ListenerSerialModem::ListenerSerialModem( QSerialPort *pSerialPort, QObject *pParent )
	: ListenerSocket( pParent ), mSerialPort( pSerialPort )
{

}

void ListenerSerialModem::processSerialRead()
{
	emit serialToConnection( mSerialPort->readAll() );
}

void ListenerSerialModem::connectionToSerial( const QString &S )
{
	int		i;

	while( ( i = S.indexOf( "+++" ) ) >= 0 )
	{
		break;
	}

	mSerialPort->write( S.toLatin1() );
}

void ListenerSerialModem::close()
{
	mSerialPort->write( "+++ATH\r\n" );

	emit disconnected( this );
}
