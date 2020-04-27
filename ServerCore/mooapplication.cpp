#include "mooapplication.h"

#include <QFile>
#include <QDateTime>
#include <QStandardPaths>
#include <QTextStream>
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
#include "taskentry.h"
#include "objectmanager.h"
#include "osc.h"

#include "listeners/listenerservertcp.h"
#include "listeners/listenerserverwebsocket.h"

#define Q(x) #x
#define QUOTE(x) Q(x)

MooApplication	*MooApplication::mInstance = nullptr;

MooApplication::MooApplication( QCoreApplication &a ) :
	QObject( &a ), mApp( a ), mMooApp( nullptr ), mOSC( nullptr ),
	mOptionDataDirectory( QStringList() << "d" << "dir", "data directory", "directory", QStandardPaths::writableLocation( QStandardPaths::AppDataLocation ) ),
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

	QFileInfo	SettingsFileInfo( optionConfigurationFile() );

	SettingsFileInfo.makeAbsolute();

	lua_moo::setSettingsFilePath( SettingsFileInfo.filePath() );

	if( !SettingsFileInfo.isFile() || !SettingsFileInfo.isReadable() )
	{
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

	//-----------------------------------------------------
	// Initalise listeners

	QSettings		Settings( MOO_SETTINGS );

	Settings.beginGroup( "listeners" );

	for( QString G : Settings.childGroups() )
	{
		Settings.beginGroup( G );

		int			Port     = Settings.value( "port" ).toInt();
		int			ObjectId = Settings.value( "object", 0 ).toInt();
		QString		Type     = Settings.value( "type" ).toString();

		if( Port < 0 || Port > 65535 )
		{
			qFatal( "Port must be in the range 0-65535" );
		}

		if( Port < 1024 )
		{
			qWarning() << "Port" << Port << "is in the 'well known' range of TCP/IP ports (0-1023)";
		}

		ListenerServer		*Server = Q_NULLPTR;

		if( Type == "tcp" )
		{
			Server = new ListenerServerTCP( ObjectId, Port );
		}
		else if( Type == "websocket" )
		{
			Server = new ListenerServerWebSocket( ObjectId, Port );
		}

		if( !Server )
		{
			return( false );
		}

		mListenerServers << Server;

		if( !CM->connection( ObjectId ) )
		{
			CM->doConnect( ObjectId );
		}

		qInfo() << "ArtMOO listening for" << Type << "connections on port" << Port;

		Settings.endGroup();
	}

	Settings.endGroup();

	if( mListenerServers.isEmpty() )
	{
		qCritical() << "No listeners defined in configuration file";

		return( false );
	}

	return( true );
}

void MooApplication::deinitialiseApp()
{
	qInfo() << "ArtMOO exiting\n";

	ConnectionManager	*CM = ConnectionManager::instance();

	CM->broadcast( "*** SERVER IS SHUTTING DOWN ***" );

	CM->disconnectAll();

	CM->processClosedSockets();

	qDeleteAll( mListenerServers );

	mListenerServers.clear();

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
