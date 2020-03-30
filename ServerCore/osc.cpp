#include "osc.h"

#include <QSettings>
#include <QtEndian>
//#include <QColor>
#include <QDateTime>

#include "objectmanager.h"
#include "lua_task.h"

QList<OSC *>		 OSC::mDeviceList;

void OSC::deviceInitialise( void )
{

}

void OSC::deviceDeinitialise( void )
{

}

void OSC::devicePacketStart( qint64 pTimeStamp )
{
	for( int i = 0 ; i < mDeviceList.size() ; i++ )
	{
		OSC *DevCur = mDeviceList[ i ];

		DevCur->packetStart( pTimeStamp );

		DevCur->processInput( pTimeStamp );
	}
}

void OSC::devicePacketEnd( qint64 pTimeStamp )
{
	static qint64	LastTime = 0;

	if( pTimeStamp - LastTime < 20 )
	{
		return;
	}

	for( int i = 0 ; i < mDeviceList.size() ; i++ )
	{
		OSC *DevCur = mDeviceList[ i ];

		DevCur->packetEnd();
	}

	LastTime = pTimeStamp;
}

void OSC::deviceCfgSave( QSettings &pDataStream )
{
	pDataStream.beginGroup( "osc" );

	pDataStream.beginWriteArray( "devices", mDeviceList.size() );

	for( int i = 0 ; i < mDeviceList.size() ; i++ )
	{
		pDataStream.setArrayIndex( i );

		OSC *DevCur = mDeviceList[ i ];

		DevCur->cfgSave( pDataStream );

		//DevCur->cfgSavePins( pDataStream );

		pDataStream.setValue( "enabled", DevCur->isEnabled() );
	}

	pDataStream.endArray();

	pDataStream.endGroup();
}

void OSC::deviceCfgLoad( QSettings &pDataStream )
{
	OSC		*DevCur;

	while( !mDeviceList.isEmpty() )
	{
		DevCur = mDeviceList.takeFirst();

		delDevice( DevCur );
	}

	pDataStream.beginGroup( "osc" );

	int		DevCnt = pDataStream.beginReadArray( "devices" );

	for( int i = 0 ; i < DevCnt ; i++ )
	{
		pDataStream.setArrayIndex( i );

		if( ( DevCur = newDevice() ) == 0 )
		{
			continue;
		}

		DevCur->cfgLoad( pDataStream );

		if( pDataStream.value( "enabled", false ).toBool() )
		{
			DevCur->setEnabled( true );
		}
	}

	pDataStream.endArray();

	pDataStream.endGroup();
}

OSC *OSC::newDevice( void )
{
	OSC		*NewDev = new OSC();

	if( NewDev != 0 )
	{
		mDeviceList.append( NewDev );
	}

	return( NewDev );
}

void OSC::delDevice( OSC *pDelDev )
{
	if( pDelDev != 0 )
	{
		mDeviceList.removeAll( pDelDev );

		delete pDelDev;
	}
}

OSC *OSC::findDevice( const QUuid &pUUID )
{
	for( int i = 0 ; i < mDeviceList.size() ; i++ )
	{
		OSC *DevCur = mDeviceList[ i ];

		if( DevCur->uuid() != pUUID )
		{
			continue;
		}

		return( DevCur );
	}

	return( 0 );
}

//-----------------------------------------------------------------------------
// Class instance

OSC::OSC( QObject *pParent ) :
	QObject( pParent ), mHostName( "localhost" ), mHostOutputPort( 7001 ), mHostInputPort( 7000 ),
	mOptionUseBundles( false ), mOptionUseInput( false ), mOptionUseOutput( true ), mEnabled( false ),
	mDeviceUuid( QUuid::createUuid() )
{
	mHostAddress = QHostAddress( QHostAddress::LocalHost );
}

void OSC::packetStart( qreal pTimeStamp )
{
	mDataInput.clear();
	mDataOutput.clear();

	mBundleTime = pTimeStamp;

	if( mSocketInput.state() != QAbstractSocket::BoundState )
	{
		return;
	}

	QByteArray		Datagram;
	QHostAddress	SendAddr;
	quint16			SendPort;

	while( mSocketInput.hasPendingDatagrams() )
	{
		Datagram.resize( mSocketInput.pendingDatagramSize() );

		mSocketInput.readDatagram( Datagram.data(), Datagram.size(), &SendAddr, &SendPort );

		if( Datagram.startsWith( "#bundle" ) )
		{
			mOptionUseBundles = true;

			processBundle( Datagram );
		}
		else
		{
			processDatagram( Datagram );
		}
	}
}

void OSC::packetEnd( void )
{
	if( mDataOutput.isEmpty() )
	{
		return;
	}

	QByteArray		Bundle;

	if( mOptionUseBundles )
	{
		Bundle.append( "#bundle\0" );		buffer( Bundle );
		Bundle.append( char( 0 ) );			buffer( Bundle );
		Bundle.append( char( 0 ) );			buffer( Bundle );
	}

	const int EmptyBundleSize = Bundle.size();

	for( QHash<QString,QVariant>::const_iterator it = mDataOutput.begin() ; it != mDataOutput.end() ; it++ )
	{
		QByteArray		Message;

		switch( it.value().type() )
		{
			case QVariant::String:
				{
					const QString		&Value = it.value().toString();

					oscMessage( Message, it.key(), "s", Value.toLatin1().constData(), Value.size() + 1 );
				}
				break;

			case QVariant::Double:
				{
					float		Value = it.value().toDouble();
					quint32		Temp, *TmpPtr;
					uchar		TmpBuf[ 4 ];

					TmpPtr = (quint32 *)&Value;
					Temp   = *TmpPtr;

					qToBigEndian( Temp, TmpBuf );

					oscMessage( Message, it.key(), "f", (const char *)&TmpBuf, 4 );
				}
				break;

//			case QVariant::Color:
//				{
//					QColor		 C = it.value().value<QColor>();
//					quint32		 i;
//					quint8		*v = (quint8 *)&i;

//					v[ 0 ] = C.red();
//					v[ 1 ] = C.green();
//					v[ 2 ] = C.blue();
//					v[ 3 ] = C.alpha();

//					i = qToBigEndian( i );

//					oscMessage( Message, it.key(), "r", (const char *)v, 4 );
//				}
//				break;

			case QVariant::Int:
				{
					uchar		TmpBuf[ 4 ];

					qToBigEndian( it.value().toInt(), TmpBuf );

					oscMessage( Message, it.key(), "i", (const char *)&TmpBuf, 4 );
				}
				break;

			default:
				continue;
		}

		if( Message.isEmpty() )
		{
			continue;
		}

		if( mOptionUseBundles )
		{
			addData( Bundle, Message );
		}
		else
		{
			mSocketOutput.writeDatagram( Message, mHostAddress, mHostOutputPort );
		}
	}

	if( Bundle.size() != EmptyBundleSize )
	{
		mSocketOutput.writeDatagram( Bundle, mHostAddress, mHostOutputPort );
	}
}

void OSC::cfgSave( QSettings &pDataStream ) const
{
	pDataStream.setValue( "name", mDeviceName );
//	pDataStream.setValue( "uuid", fugio::utils::uuid2string( mDeviceUuid ) );

	pDataStream.setValue( "address", mHostName );

	pDataStream.setValue( "port-input",  mHostInputPort );
	pDataStream.setValue( "port-output", mHostOutputPort );

	pDataStream.setValue( "use-bundles", mOptionUseBundles );
	pDataStream.setValue( "use-input", mOptionUseInput );
	pDataStream.setValue( "use-output", mOptionUseOutput );

	pDataStream.setValue( "enabled", mEnabled );
}

void OSC::cfgLoad( QSettings &pDataStream )
{
	setPath( pDataStream.value( "name", mDeviceName ).toString() );

//	mDeviceUuid = fugio::utils::string2uuid( pDataStream.value( "uuid", fugio::utils::uuid2string( mDeviceUuid ) ).toString() );

	setHostName( pDataStream.value( "address", mHostName ).toString() );

	mHostInputPort  = pDataStream.value( "port-input",  mHostInputPort  ).toUInt();
	mHostOutputPort = pDataStream.value( "port-output", mHostOutputPort ).toUInt();

	mOptionUseBundles = pDataStream.value( "use-bundles", mOptionUseBundles ).toBool();

	mOptionUseInput = pDataStream.value( "use-input", mOptionUseInput ).toBool();
	mOptionUseOutput = pDataStream.value( "use-output", mOptionUseOutput ).toBool();

	mEnabled = pDataStream.value( "enabled", mEnabled ).toBool();
}

bool OSC::isEnabled() const
{
	return( mEnabled );
}

void OSC::setEnabled( bool pEnabled )
{
	if( pEnabled )
	{
		mSocketInput.close();

		if( mOptionUseInput && !mSocketInput.bind( mHostInputPort ) )
		{
			qWarning() << tr( "OSC Connection Error" ) << QString( tr( "Can't connect to port %1" ) ).arg( mHostInputPort );
		}
	}
	else
	{
		mSocketInput.close();
	}

	mEnabled = pEnabled;
}

void OSC::output( const QString &pAddress, const QVariant &pValue )
{
	if( mOptionUseOutput )
	{
		mDataOutput.insert( pAddress, pValue );
	}
}

void OSC::setHostName( const QString &pAddress )
{
	mHostName = pAddress;

	mHostAddress = QHostAddress();

	QHostInfo::lookupHost( pAddress, this, SLOT(outputAddressResolved(QHostInfo)) );
}

void OSC::outputAddressResolved( QHostInfo pHostInfo )
{
	if( pHostInfo.error() != QHostInfo::NoError )
	{
		qWarning() << tr( "OSC Host" ) << pHostInfo.errorString();
	}
	else if( pHostInfo.addresses().isEmpty() )
	{

	}
	else
	{
		for( int i = 0 ; i < pHostInfo.addresses().size() ; i++ )
		{
			const QHostAddress	&HA = pHostInfo.addresses().at( i );

			if( HA.protocol() != QAbstractSocket::IPv4Protocol )
			{
				continue;
			}

			mHostAddress = HA;

			break;
		}
	}
}

void OSC::processDatagram( const QByteArray &pDatagram )
{
	QByteArray		OscAdr;
	int				OscStr = 0;
	int				OscEnd = 0;

	OscEnd = pDatagram.indexOf( (char)0x00, OscStr );
	OscAdr = pDatagram.mid( OscStr, OscEnd - OscStr );
	OscStr = OscEnd + ( 4 - ( OscEnd % 4 ) );

	if( OscAdr.at( 0 ) != '/' )
	{
		return;
	}

	//OscAdr.remove( 0, 1 );

	QByteArray		OscArg;

	OscEnd = pDatagram.indexOf( (char)0x00, OscStr );
	OscArg = pDatagram.mid( OscStr, OscEnd - OscStr );
	OscStr = OscEnd + ( 4 - ( OscEnd % 4 ) );

	if( OscArg.at( 0 ) != ',' )
	{
		return;
	}

	OscArg.remove( 0, 1 );

	QVariantList	OscLst;

	for( int a = 0 ; a < OscArg.size() ; a++ )
	{
		int		OscInc = 0;

		switch( OscArg.at( a ) )
		{
			case 'i':	// int32
				{
					qint32		v;

					memcpy( &v, pDatagram.data() + OscStr, sizeof( v ) );

					v = qFromBigEndian( v );

					OscLst.append( v );

					OscInc = sizeof( v );
				}
				break;

			case 'f':	// float32
				{
					qint32		v;

					memcpy( &v, pDatagram.data() + OscStr, sizeof( v ) );

					v = qFromBigEndian( v );

					float		*f = (float *)&v;

					OscLst.append( *f );

					OscInc = sizeof( v );
				}
				break;

			case 's':	// OSC-string
				{
					QByteArray		OscVar;

					OscEnd = pDatagram.indexOf( (char)0x00, OscStr );
					OscVar = pDatagram.mid( OscStr, OscEnd - OscStr );
					OscStr = OscEnd + ( 4 - ( OscEnd % 4 ) );

					OscLst.append( QString( OscVar ) );
				}
				break;

			case 'b':	// OSC-blob
				{

				}
				break;

			case 'h':	// 64 bit big-endian two's complement integer
				{
					qlonglong		v;

					memcpy( &v, pDatagram.data() + OscStr, sizeof( v ) );

					v = qFromBigEndian( v );

					OscLst.append( v );

					OscInc = sizeof( v );
				}
				break;

			case 't':	// OSC-timetag
				break;

			case 'd':	// 64 bit ("double") IEEE 754 floating point number
				{
					qint64		v;

					memcpy( &v, pDatagram.data() + OscStr, sizeof( v ) );

					v = qFromBigEndian( v );

					double		*f = (double *)&v;

					OscLst.append( *f );

					OscInc = sizeof( v );
				}
				break;

			case 'S':	// Alternate type represented as an OSC-string (for example, for systems that differentiate "symbols" from "strings")
				break;

			case 'c':	// an ascii character, sent as 32 bits
				{
					qint32		v;

					memcpy( &v, pDatagram.data() + OscStr, sizeof( v ) );

					v = qFromBigEndian( v );

					OscLst.append( QChar( v ) );

					OscInc = sizeof( v );
				}
				break;

//			case 'r':	// 32 bit RGBA color
//				{
//					qint32		v;

//					memcpy( &v, pDatagram.data() + OscStr, sizeof( v ) );

//					v = qFromBigEndian( v );

//					const quint8 *p = (const quint8 *)&v;

//					OscLst.append( QColor( p[ 0 ], p[ 1 ], p[ 2 ], p[ 3 ] ) );

//					OscInc = sizeof( v );
//				}
//				break;

			case 'm':	// 4 byte MIDI message. Bytes from MSB to LSB are: port id, status byte, data1, data2
				break;

			case 'T':	// True. No bytes are allocated in the argument data.
				OscLst.append( true );
				break;

			case 'F':	// False. No bytes are allocated in the argument data.
				OscLst.append( false );
				break;

			case 'N':	// Nil. No bytes are allocated in the argument data.
				break;

			case 'I':	// Infinitum. No bytes are allocated in the argument data.
				break;

			case '[':	// Indicates the beginning of an array. The tags following are for data in the Array until a close brace tag is reached.
				break;

			case ']':	// Indicates the end of an array.
				break;

			default:
				break;
		}

		OscStr += OscInc;
		OscEnd += OscInc;
	}

	if( OscLst.size() == 0 )
	{
		mDataInput.remove( OscAdr );
	}
	else if( OscLst.size() == 1 )
	{
		mDataInput.insert( OscAdr, OscLst.first() );
		mDataNames.insert( OscAdr, OscLst.first() );
	}
	else
	{
		mDataInput.insert( OscAdr, OscLst );
		mDataNames.insert( OscAdr, OscLst );
	}
}

void OSC::processBundle( const QByteArray &pDatagram )
{
	QByteArray		OscAdr;
	int				OscStr = 0;
	int				OscEnd = 0;

	OscEnd = pDatagram.indexOf( (char)0x00, OscStr );
	OscAdr = pDatagram.mid( OscStr, OscEnd - OscStr );
	OscStr = OscEnd + ( 4 - ( OscEnd % 4 ) );

	quint64		TimeTag;

	memcpy( &TimeTag, pDatagram.data() + OscStr, 8 );

	OscStr += 8;

	TimeTag = qFromBigEndian( TimeTag );

	while( OscStr < pDatagram.size() )
	{
		quint32		ElementSize;

		memcpy( &ElementSize, pDatagram.data() + OscStr, 4 );

		ElementSize = qFromBigEndian( ElementSize );

		OscStr += 4;

		processDatagram( pDatagram.mid( OscStr, ElementSize ) );

		OscStr += ElementSize;
	}
}

void OSC::sendData( const QString &pPath, const QVariant &pValue )
{
	if( mOptionUseOutput )
	{
		mDataOutput.insert( pPath, pValue );
	}
}

void OSC::buffer( QByteArray &pArray )
{
	while( pArray.size() % 4 != 0 )
	{
		pArray.append( char( 0x00 ) );
	}
}

void OSC::addData( QByteArray &pBundle, const QString &pName, const QString &pArgs, const char *pBuffer, qint32 pBufLen)
{
	QByteArray		Packet;

	oscMessage( Packet, pName, pArgs, pBuffer, pBufLen );

	quint32			Temp32 = Packet.size();
	uchar			TmpBuf[ 4 ];

	qToBigEndian( Temp32, TmpBuf );

	pBundle.append( (const char *)&TmpBuf, 4 );		buffer( pBundle );

	pBundle.append( Packet );
}

void OSC::addData( QByteArray &pBundle, const QByteArray &pPacket )
{
	quint32			Temp32 = pPacket.size();
	uchar			TmpBuf[ 4 ];

	qToBigEndian( Temp32, TmpBuf );

	pBundle.append( (const char *)&TmpBuf, 4 );		buffer( pBundle );

	pBundle.append( pPacket );
}

void OSC::oscMessage( QByteArray &pMessage, const QString &pName, const QString &pArgs, const char *pBuffer, qint32 pBufLen )
{
	pMessage.append( pName );
	pMessage.append( char( 0 ) );				buffer( pMessage );
	pMessage.append( ',' );
	pMessage.append( pArgs );
	pMessage.append( char( 0 ) );				buffer( pMessage );
	pMessage.append( pBuffer, pBufLen );		buffer( pMessage );
}

void OSC::processInput( qint64 pTimeStamp )
{
	for( OSC *DEV : OSC::devices() )
	{
		const QHash<QString,QVariant>	OSCData = DEV->data();

		for( auto it = OSCData.begin() ; it != OSCData.end() ; it++ )
		{
			QStringList		 Path = it.key().split( '/', QString::SkipEmptyParts );

			Object		*O = ObjectManager::instance()->rootObject();

			while( O && !Path.isEmpty() )
			{
				if( Path.size() == 1 )
				{
					// TODO: Security

					if( Property *P = O->prop( Path.at( 1 ) ) )
					{
						P->setValue( it.value() );
					}
					else
					{
						Task		T;

						T.setObject( *O );
						T.setTimeStamp( pTimeStamp );
						T.setArgStr( it.value().toString() );

						lua_task		L( -1, T );

						L.execute( pTimeStamp );
					}
				}
				else
				{
					Object	*C = nullptr;

					for( ObjectId OID : O->children() )
					{
						Object	*T = ObjectManager::o( OID );

						if( T && T->name() == Path.first() )
						{
							C = T;

							break;
						}
					}

					O = C;
				}

			   Path.removeFirst();
			}
		}
	}
}

