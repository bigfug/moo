#include <QCoreApplication>

#include "mooapp.h"
#include "connectionmanager.h"
#include "connection.h"
#include "listenertelnet.h"
#include "listenerwebsocket.h"
#include "taskentry.h"
#include <lua.hpp>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include "objectmanager.h"
#include "osc.h"

void myMessageOutput( QtMsgType type, const QMessageLogContext &context, const QString &msg )
{
	Q_UNUSED( context )

	QFile				LogFil( "moo.log" );
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

	LogMsg = QString( "%1 - %2: %3" ).arg( DateTime ).arg( LogMsg ).arg( msg );

	if( LogFil.open( QIODevice::Append | QIODevice::Text ) )
	{
		QTextStream		TxtOut( &LogFil );

		TxtOut << LogMsg << "\n";

		LogFil.close();
	}

#if defined( QT_DEBUG )
	QTextStream out( stdout, QIODevice::WriteOnly );

	out << msg << "\n";
#endif
}

int main( int argc, char *argv[] )
{
	QCoreApplication	a( argc, argv );

	a.setApplicationName( "ArtMOO" );
	a.setOrganizationName( "Alex May" );
	a.setOrganizationDomain( "bigfug.com" );
	a.setApplicationVersion( "0.1" );

	QString			DataDir    = ".";
	quint16			ServerPort = 1123;

	const QStringList	args = a.arguments();

	foreach( const QString &arg, args )
	{
		if( arg.startsWith( "-dir=", Qt::CaseSensitive ) )
		{
			DataDir = arg.mid( 5 );

			continue;
		}

		if( arg.startsWith( "-port=", Qt::CaseInsensitive ) )
		{
			ServerPort = arg.mid( 6 ).toInt();

			continue;
		}
	}

	if( DataDir != "." )
	{
		if( !QDir::setCurrent( DataDir ) )
		{
			qFatal( QString( QObject::tr( "Can't set directory to %1" ) ).arg( DataDir ).toLatin1() );
		}
	}

	qInstallMessageHandler( myMessageOutput );

	qDebug() << "ArtMOO v0.1 by Alex May - www.bigfug.com";

	int				 Ret = -1;

	mooApp			*App = new mooApp();

	if( App != 0 )
	{
		OSC			*OSCHost = OSC::newDevice();

		if( OSCHost )
		{
			ConnectionManager	*CM = ConnectionManager::instance();

			if( CM != 0 )
			{
				ListenerServer	*L = new ListenerTelnet( 0, ServerPort, CM );

				if( L != 0 )
				{
					CM->doConnect( 0 );

					qDebug() << "ArtMOO listening for connections on port" << ServerPort;

					Ret = a.exec();

					qDebug() << "ArtMOO exiting\n";

					delete L;
				}

				delete CM;
			}

			OSC::delDevice( OSCHost );
		}

		delete App;
	}

	return( Ret );
}
