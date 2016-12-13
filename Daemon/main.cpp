#include <QCoreApplication>

#include "mooapp.h"
#include "connectionmanager.h"
#include "connection.h"
#include "listener.h"
#include "taskentry.h"
#include <lua.hpp>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include "objectmanager.h"
#include "osc.h"

QString		LogFileName;

void myMessageOutput( QtMsgType type, const QMessageLogContext &context, const QString &msg )
{
	Q_UNUSED( context )

	QFile				LogFil( LogFileName );
	const QDateTime		CurDat = QDateTime::currentDateTime();
	QString				DateTime = CurDat.toString( "dd-MMM-yyyy hh:mm:ss" );
	QString				LogMsg;

	switch( type )
	{
		default:
			LogMsg = QString( "%1 - %2" ).arg( DateTime ).arg( msg );
	}

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
	a.setOrganizationDomain( "http://www.bigfug.com" );
	a.setApplicationVersion( "0.1" );

	QString			DataFileName = "moo.dat";
	quint16			ServerPort   = 1123;

	const QStringList	args = a.arguments();

	foreach( const QString &arg, args )
	{
		if( arg.startsWith( "-db=", Qt::CaseSensitive ) )
		{
			DataFileName = arg.mid( 4 );

			continue;
		}

		if( arg.startsWith( "-port=", Qt::CaseInsensitive ) )
		{
			ServerPort = arg.mid( 6 ).toInt();

			continue;
		}
	}

	QFileInfo	LogFileInfo = QFileInfo( DataFileName );

	LogFileName = LogFileInfo.path() + "/" + LogFileInfo.completeBaseName() + ".log";

	qInstallMessageHandler( myMessageOutput );

	qDebug() << "ArtMOO v0.1 by Alex May - www.bigfug.com";

	int				 Ret = -1;

	mooApp			*App = new mooApp( DataFileName );

	if( App != 0 )
	{
		OSC			*OSCHost = OSC::newDevice();

		if( OSCHost )
		{
			ConnectionManager	*CM = ConnectionManager::instance();

			if( CM != 0 )
			{
				Listener	*L = new Listener( 0, ServerPort, CM );

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
