#include <QDateTime>
#include <QFileInfo>

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

#include "osc.h"

#include "odb_file.h"
#include "odb_sql.h"

mooApp::mooApp( const QString &pDataFileName, QObject *pParent )
	: QObject( pParent ), mTimerId( 0 ), mDataFileName( pDataFileName )
{
	connect( this, SIGNAL(destroyed(QObject*)), this, SLOT(cleanup(QObject*)));

	lua_moo::initialiseAll();

	ObjectManager		&OM = *ObjectManager::instance();

	OSC::deviceInitialise();

	bool LoadFromFile = !QFileInfo( "moo.sql" ).exists();

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

	if( OM.object( 0 ) != 0 )
	{
		QList<Object *>		ObjLst = ObjectManager::instance()->connectedPlayers();

		for( Object *O : ObjLst )
		{
			QString			 CMD = QString( "moo.root:user_disconnected( o( %1 ) )" ).arg( O->id() );
			TaskEntry		 TE( CMD, 0, 0 );
			lua_task		 Com( 0, TE );

			Com.eval();

			O->setConnection( -1 );
		}

		if( true )
		{
			QString			 CMD = QString( "moo.root:server_started()" );
			TaskEntry		 TE( CMD, 0, 0 );
			lua_task		 Com( 0, TE );

			Com.eval();
		}
	}

	mTimerId = startTimer( 40 );
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

//	ODBFile		ODB( mDataFileName );

//	ODB.save();

//	if
//	ODBSQL		SQL;

//	SQL.save();

	ObjectManager::instance()->reset();

	OSC::deviceDeinitialise();
}

void mooApp::doOutput( const QString &pText )
{
	emit textOutput( pText );
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

