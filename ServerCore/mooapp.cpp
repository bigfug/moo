#include <QDateTime>
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

mooApp::mooApp( const QString &pDataFileName, QObject *pParent )
	: QObject( pParent ), mTimerId( 0 ), mDataFileName( pDataFileName )
{
	connect( this, SIGNAL(destroyed(QObject*)), this, SLOT(cleanup(QObject*)));

	lua_moo::initialiseAll();

	ObjectManager		&OM = *ObjectManager::instance();

	OM.load( mDataFileName );

	if( OM.maxId() == 0 )
	{
		OM.luaMinimal();
	}

	if( OM.object( 0 ) != 0 )
	{
		QString			 CMD = QString( "moo.root:server_started()" );
		TaskEntry		 TE( CMD, 0, 0 );
		lua_task		 Com( 0, TE );

		Com.eval();
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

	ObjectManager::instance()->save( mDataFileName );

	ObjectManager::instance()->reset();
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

	ObjectManager::instance()->onFrame( QDateTime::currentMSecsSinceEpoch() );
}

void mooApp::doTask( TaskEntry &pTask )
{
	ObjectManager::instance()->doTask( pTask );
}

