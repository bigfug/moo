#include "mooapp.h"
#include "mainwindow.h"
#include "connectionmanager.h"
#include "connection.h"
#include "listenertelnet.h"
#include "listenerwebsocket.h"
#include "taskentry.h"
#include <QApplication>
#include <lua.hpp>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QDebug>

#include "objectmanager.h"
#include "osc.h"

MainWindow		*gMainWindow = Q_NULLPTR;

void myMessageOutput( QtMsgType type, const QMessageLogContext &context, const QString &msg )
{
	Q_UNUSED( context )

	QFile				LogFil( "moo.log" );
	const QDateTime		CurDat = QDateTime::currentDateTime();
	QString				DateTime = CurDat.toString( "dd-MMM-yyyy hh:mm:ss" );
	QString				LogMsg;

	switch( type )
	{
		default:
			LogMsg = QString( "%1 - %2" ).arg( DateTime ).arg( msg );
	}

	if( gMainWindow )
	{
		gMainWindow->log( LogMsg );
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
	QApplication	a( argc, argv );

	a.setApplicationName( "ArtMOO" );
	a.setOrganizationName( "Alex May" );
	a.setOrganizationDomain( "bigfug.com" );
	a.setApplicationVersion( "0.1" );

	MainWindow		w;

	gMainWindow = &w;

	qInstallMessageHandler( myMessageOutput );

	w.show();

	qDebug() << "ArtMOO v0.1 by Alex May - www.bigfug.com";

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
			ServerPort = arg.mid( 6 ).toUShort();

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

	int				 Ret = -1;

	mooApp			*App = new mooApp();

	if( App )
	{
		QObject::connect( ObjectManager::instance(), SIGNAL(stats(ObjectManagerStats)), &w, SLOT(stats(ObjectManagerStats)) );

		OSC			*OSCHost = OSC::newDevice();

		if( OSCHost )
		{
			ConnectionManager	*CM = ConnectionManager::instance();

			if( CM )
			{
				ListenerServer	*L = new ListenerTelnet( 0, ServerPort, CM );

				if( L )
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
