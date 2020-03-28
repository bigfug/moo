#include "mooapplication.h"

#include <QFile>
#include <QDateTime>
#include <QStandardPaths>
#include <QTextStream>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QSysInfo>

#include <iostream>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "lua_moo.h"
#include "mooapp.h"

#include "connectionmanager.h"
#include "connection.h"
#include "listenertelnet.h"
#include "listenerwebsocket.h"
#include "taskentry.h"
#include "objectmanager.h"
#include "osc.h"

#define Q(x) #x
#define QUOTE(x) Q(x)

MooApplication	*MooApplication::mInstance = nullptr;

MooApplication::MooApplication( QCoreApplication &a ) :
	QObject( &a ), mApp( a ), mMooApp( nullptr ), mOSC( nullptr ), mListenerTelnet( nullptr ),
	mOptionDataDirectory( QStringList() << "d" << "dir", "data directory", "directory", QStandardPaths::writableLocation( QStandardPaths::AppDataLocation ) ),
	mOptionServerPort( QStringList() << "p" << "port", "default server port", "port", "1123" ),
	mOptionConfiguration( QStringList() << "c" << "cfg", "path to moo.ini", "filepath", "moo.ini" )
{
	mInstance = this;

	a.setApplicationName( "ArtMOO" );
	a.setOrganizationName( "Alex May" );
	a.setOrganizationDomain( "www.bigfug.com" );

#if QT_VERSION < QT_VERSION_CHECK( 5, 4, 0 )
	QApplication::setApplicationVersion( QUOTE( MOO_VERSION ) );
#else
	a.setApplicationVersion( QString( "%1 (%2/%3)" ).arg( QUOTE( MOO_VERSION ) ).arg( QSysInfo::buildCpuArchitecture() ).arg( QSysInfo::currentCpuArchitecture() ) );
#endif

	connect( this, &MooApplication::message, this, &MooApplication::fileMessageHandler );

#if defined( QT_DEBUG )
	connect( this, &MooApplication::message, this, &MooApplication::debugMessageHandler );
#endif

	qInstallMessageHandler( &MooApplication::staticMessageHandler );

	//-------------------------------------------------------------------------
	// Command line parsing

	mParser.setApplicationDescription( "ArtMOO" );
	mParser.addHelpOption();
	mParser.addVersionOption();

	mParser.addOption( mOptionDataDirectory );
	mParser.addOption( mOptionServerPort );
	mParser.addOption( mOptionConfiguration );
}

MooApplication::~MooApplication()
{
	qInstallMessageHandler( nullptr );

	mInstance = 0;
}

void MooApplication::process( void )
{
	qInfo() << QString( "ArtMOO v%1 by Alex May - www.bigfug.com" ).arg( mApp.applicationVersion() );

	mParser.process( mApp );

	//-------------------------------------------------------------------------

	lua_moo::setSettingsFilePath( optionConfigurationFile() );

	QFileInfo	SettingsFileInfo( lua_moo::settingsFilePath() );

	if( !SettingsFileInfo.isFile() || !SettingsFileInfo.isReadable() )
	{
		SettingsFileInfo.makeAbsolute();

		qCritical() << "Can't open" << SettingsFileInfo.filePath();
	}
	else
	{
		SettingsFileInfo.makeAbsolute();

		qInfo() << "Using settings in" << SettingsFileInfo.filePath();
	}

	//-------------------------------------------------------------------------

	QFileInfo	DataDirInfo( mParser.value( mOptionDataDirectory ) );

	if( !DataDirInfo.exists() )
	{
		DataDirInfo.dir().mkpath( DataDirInfo.fileName() );
	}

	if( !QDir::setCurrent( DataDirInfo.filePath() ) )
	{
		qFatal( "Can't set directory to %s", qPrintable( DataDirInfo.filePath() ) );
	}

	qInfo() << "Using data directory" << DataDirInfo.filePath();

	//-------------------------------------------------------------------------

	int			Port = optionServerPort();

	if( Port < 0 || Port > 65535 )
	{
		qFatal( "Port must be in the range 0-65535" );
	}

	if( Port < 1024 )
	{
		qWarning() << "Port" << Port << "is in the 'well known' range of TCP/IP ports (0-1023)";
	}
}

bool MooApplication::initialiseApp()
{
	ConnectionManager	*CM = ConnectionManager::instance();

	if( !CM )
	{
		return( false );
	}

	mMooApp = new mooApp();

	if( !mMooApp )
	{
		return( false );
	}

	mOSC = OSC::newDevice();

	if( !mOSC )
	{
		return( false );
	}

	mListenerTelnet	= new ListenerTelnet( 0, optionServerPort() );

	if( !mListenerTelnet )
	{
		return( false );
	}

	CM->doConnect( 0 );

	qInfo() << "ArtMOO listening for connections on port" << optionServerPort();

	return( true );
}

void MooApplication::deinitialiseApp()
{
	qInfo() << "ArtMOO exiting\n";

	ConnectionManager	*CM = ConnectionManager::instance();

	CM->broadcast( "*** SERVER IS SHUTTING DOWN ***" );

	CM->disconnectAll();

	CM->processClosedSockets();

	if( mListenerTelnet )
	{
		delete mListenerTelnet;

		mListenerTelnet = nullptr;
	}

	if( mOSC )
	{
		OSC::delDevice( mOSC );

		mOSC = nullptr;
	}

	if( mMooApp )
	{
		delete mMooApp;

		mMooApp = nullptr;
	}

	ConnectionManager::reset();
}

void MooApplication::staticMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	if( mInstance )
	{
		mInstance->messageHandler( type, context, msg );
	}
}

void MooApplication::messageHandler( QtMsgType type, const QMessageLogContext &context, const QString &msg )
{
	Q_UNUSED( context )

	const QDateTime		CurDat = QDateTime::currentDateTime();
	QString				DateTime = CurDat.toString( "dd-MMM-yyyy hh:mm:ss" );
	QString				LogMsg;

	switch( type )
	{
		case QtDebugMsg:
			LogMsg = "[DBUG]";
			break;

		case QtInfoMsg:
			LogMsg = "[INFO]";
			break;

		case QtWarningMsg:
			LogMsg = "[WARN]";
			break;

		case QtCriticalMsg:
			LogMsg = "[CRIT]";
			break;

		default:
			LogMsg = "[----]";
			break;
	}

	emit message( QString( "%1 - %2: %3" ).arg( DateTime ).arg( LogMsg ).arg( msg ) );
}

void MooApplication::fileMessageHandler(QString pMessage)
{
	QFile				LogFil( "moo.log" );

	if( LogFil.open( QIODevice::Append | QIODevice::Text ) )
	{
		QTextStream		TxtOut( &LogFil );

		TxtOut << pMessage << "\n";

		LogFil.close();
	}
}

void MooApplication::debugMessageHandler(QString pMessage)
{
	std::cout << pMessage.toStdString() << std::endl;
}
