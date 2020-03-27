#include <QDateTime>
#include <QFileInfo>
#include <QtGlobal>
#include <QSettings>

#include "mooapp.h"
#include "object.h"
#include "objectmanager.h"
#include "connectionmanager.h"
#include "task.h"
#include "connection.h"
#include "lua_moo.h"
#include "lua_object.h"
#include "lua_connection.h"
#include "lua_prop.h"
#include "lua_verb.h"
#include "lua_task.h"
#include <iostream>
#include <algorithm>
#include <QDateTime>
#include <QTimer>

#include "osc.h"

#include "odb_file.h"
#include "odb_sql.h"

mooApp::mooApp( const QString &pDataFileName, QObject *pParent )
	: QObject( pParent ), mTimerId( 0 ), mDataFileName( pDataFileName )
{
	//QSettings::setDefaultFormat( QSettings::IniFormat );

	connect( this, SIGNAL(destroyed(QObject*)), this, SLOT(cleanup(QObject*)));

	lua_moo::initialiseAll();

	ObjectManager		&OM = *ObjectManager::instance();

	OSC::deviceInitialise();

	bool LoadFromFile = !QFileInfo( "moo.db" ).exists();

	ODBSQL		*SQL = new ODBSQL();

	if( SQL )
	{
		OM.setODB( SQL );
	}

	if( LoadFromFile )
	{
		ODBFile		ODB( mDataFileName );

		ODB.load();

		SQL->save();
	}
	else
	{
		SQL->load();
	}

	if( OM.maxId() == 0 )
	{
		OM.luaMinimal();
	}

	Object		*System = OM.systemObject();

	if( System )
	{
		QList<Object *>		ObjLst = ObjectManager::instance()->connectedPlayers();

		bool		HasVerb = System->verb( "user_disconnected" );

		for( Object *O : ObjLst )
		{
			if( HasVerb )
			{
				lua_task::process( QString( "moo.system:user_disconnected( o( %1 ) )" ).arg( O->id() ) );
			}

			O->setConnection( -1 );
		}

		if( System->verb( "server_started" ) )
		{
			lua_task::process( "moo.system:server_started()" );
		}
	}

	qint64		TimeToNextTask = OM.timeToNextTask();

	if( TimeToNextTask >= 0 )
	{
		qDebug() << "Time to next task:" << TimeToNextTask;
	}

	connect( &mTimer, SIGNAL(timeout()), this, SLOT(taskReady()) );

	mTimer.setSingleShot( true );

	connect( &OM, SIGNAL(taskReady()), this, SLOT(taskReady()), Qt::QueuedConnection );

	taskReady();
}

void mooApp::cleanup( QObject *pObject )
{
	Q_UNUSED( pObject )
}

mooApp::~mooApp()
{
	if( mTimerId == 0 )
	{
		killTimer( mTimerId );
	}

	Object		*System = ObjectManager::instance()->systemObject();

	if( System && System->verb( "server_closing" ) )
	{
		lua_task::process( "moo.system:server_closing()" );
	}

	ODB			*OM_ODB = ObjectManager::instance()->odb();

	if( OM_ODB )
	{
		OM_ODB->save();
	}

	ConnectionManager::reset();

	ObjectManager::reset();

	OSC::deviceDeinitialise();
}

void mooApp::doOutput( const QString &pText )
{
	emit textOutput( pText );
}

void mooApp::taskReady()
{
	ObjectManager		&OM = *ObjectManager::instance();

//	mTimer.stop();

	qint64	TimeStamp = QDateTime::currentMSecsSinceEpoch();

	OSC::devicePacketStart( TimeStamp );

	emit frameStart();
	emit frameStart( TimeStamp );

	OM.onFrame( TimeStamp );

	emit frameEnd();
	emit frameEnd( TimeStamp );

	OSC::devicePacketEnd( TimeStamp );

	qint64		TimeToNext = OM.timeToNextTask();

	if( TimeToNext > 0 )
	{
		mTimer.setInterval( qMin( 1000LL, TimeToNext ) );
		mTimer.start();
	}
	else
	{
		mTimer.setInterval( 1000 );
		mTimer.start();
	}
}

void mooApp::streamCallback( const QString &pText, void *pUserData )
{
	((mooApp*)pUserData)->doOutput( pText );
}

void mooApp::timerEvent( QTimerEvent *pEvent )
{
	Q_UNUSED( pEvent )

	qint64	TimeStamp = QDateTime::currentMSecsSinceEpoch();

	OSC::devicePacketStart( TimeStamp );

	emit frameStart();
	emit frameStart( TimeStamp );

	ObjectManager::instance()->onFrame( TimeStamp );

	emit frameEnd();
	emit frameEnd( TimeStamp );

	OSC::devicePacketEnd( TimeStamp );
}

void mooApp::doTask( TaskEntry &pTask )
{
	ObjectManager::instance()->doTask( pTask );
}

