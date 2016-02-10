#ifndef OSC_H
#define OSC_H

#include <QObject>
#include <QUdpSocket>
#include <QHash>
#include <QHostAddress>
#include <QHostInfo>
#include <QSettings>
#include <QUuid>

class OSC : public QObject
{
	Q_OBJECT

public:
	static void deviceInitialise( void );
	static void deviceDeinitialise( void );
	static void devicePacketStart( qint64 pTimeStamp );
	static void devicePacketEnd( qint64 pTimeStamp );

	static void deviceCfgSave( QSettings &pDataStream );
	static void deviceCfgLoad( QSettings &pDataStream );

	static OSC *newDevice( void );

	static void delDevice( OSC *pDelDev );

	static QList<OSC *> devices( void )
	{
		return( mDeviceList );
	}

	static OSC *findDevice( const QUuid &pUUID );

private:
	explicit OSC( QObject *pParent = 0 );

	virtual ~OSC( void ) {}

public:
	virtual void packetStart( qreal pTimeStamp );
	virtual void packetEnd( void );

	virtual void cfgSave( QSettings &pDataStream ) const;
	virtual void cfgLoad( QSettings &pDataStream );

	virtual bool isEnabled( void ) const;
	virtual void setEnabled( bool pEnabled );

	inline QString hostName( void ) const
	{
		return( mHostName );
	}

	inline quint16 portInput( void ) const
	{
		return( mHostInputPort );
	}

	inline quint16 portOutput( void ) const
	{
		return( mHostOutputPort );
	}

	inline bool optionUseBundles( void ) const
	{
		return( mOptionUseBundles );
	}

	inline bool optionUseInput( void ) const
	{
		return( mOptionUseInput );
	}

	inline bool optionUseOutput( void ) const
	{
		return( mOptionUseOutput );
	}

	inline const QHash<QString,QVariant> &data( void ) const
	{
		return( mDataInput );
	}

	inline const QHash<QString,QVariant> &names( void ) const
	{
		return( mDataNames );
	}

	inline QString name( void ) const
	{
		return( mDeviceName );
	}

	inline QUuid uuid( void ) const
	{
		return( mDeviceUuid );
	}

	void output( const QString &pAddress, const QVariant &pValue );

private:
	static void buffer( QByteArray &pArray );

	static void addData( QByteArray &pBundle, const QString &pName, const QString &pArgs, const char *pBuffer, qint32 pBufLen );

	void addData( QByteArray &pBundle, const QByteArray &pPacket );

	static void oscMessage( QByteArray &pMessage, const QString &pName, const QString &pArgs, const char *pBuffer, qint32 pBufLen );

	void processDatagram( const QByteArray &pDatagram );

	void processBundle( const QByteArray &pDatagram );

	void processInput( qint64 pTimeStamp );

public slots:
	void sendData( const QString &pPath, const QVariant &pValue );

	void setHostName( const QString &pAddress );

	void setPortInput( quint16 pPort )
	{
		mHostInputPort = pPort;
	}

	void setPortOutput( quint16 pPort )
	{
		mHostOutputPort = pPort;
	}

	void setOptionUseBundles( bool pOption )
	{
		mOptionUseBundles = pOption;
	}

	void setOptionUseInput( bool pOption )
	{
		mOptionUseInput = pOption;
	}

	void setOptionUseOutput( bool pOption )
	{
		mOptionUseOutput = pOption;
	}

	void setPath( const QString &pName )
	{
		mDeviceName = pName;
	}

private slots:
	void outputAddressResolved( QHostInfo pHostInfo );

private:
	QUdpSocket					 mSocketOutput;
	QUdpSocket					 mSocketInput;
	QHash<QString,QVariant>		 mDataNames;
	QHash<QString,QVariant>		 mDataInput;
	QHash<QString,QVariant>		 mDataOutput;
	qreal						 mBundleTime;
	QString						 mHostName;
	QHostAddress				 mHostAddress;
	quint16						 mHostOutputPort;
	quint16						 mHostInputPort;
	bool						 mOptionUseBundles;
	bool						 mOptionUseInput;
	bool						 mOptionUseOutput;
	bool						 mEnabled;
	QString						 mDeviceName;
	QUuid						 mDeviceUuid;

	static QList<OSC *>			 mDeviceList;
};

#endif // OSC_H
