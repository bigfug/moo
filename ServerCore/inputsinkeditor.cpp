#include "inputsinkeditor.h"

#include <QDebug>

#include "connection.h"
#include "object.h"
#include "verb.h"
#include "lua_moo.h"
#include <lua.hpp>

InputSinkEditor::InputSinkEditor(Connection *C, Object *O, Verb *V, const QString &pVerbName, QStringList pText )
	: mConnection( C ), mObject( O ), mVerb( V ), mVerbName( pVerbName )
{
	mConnection->setLineMode( Connection::REALTIME );

	mEditor.setSize( mConnection->terminalSize() );

	connect( &mEditor, SIGNAL(output(QString)), this, SLOT(output(QString)) );

	mEditor.setText( pText );

	mEditor.redraw();

	mEditor.addControlSlot( 20, this, "test" );
	mEditor.addControlSlot( 19, this, "save" );
}

bool InputSinkEditor::input( const QString &pData )
{
	mEditor.input( pData );

	if( mEditor.hasQuit() )
	{
		mConnection->notify( "\e[2J\e[1;1H" );

		mConnection->setLineMode( Connection::EDIT );
	}

	return( !mEditor.hasQuit() );
}

void InputSinkEditor::output( const QString &pData )
{
	qDebug() << "InputSinkEditor::output" << pData;

	mConnection->notify( pData );
}

void InputSinkEditor::test()
{
	lua_State		*L = luaL_newstate();

	lua_moo::luaNewState( L );

	QByteArray		 P = mEditor.text().join( "\n" ).toUtf8();

	int Error = luaL_loadbuffer( L, P, P.size(), mVerbName.toUtf8() );

	if( Error == 0 )
	{
		mEditor.setStatusMessage( tr( "No errors detected" ) );
	}
	else
	{
		QString		Result = lua_tostring( L, -1 );

		mEditor.setStatusMessage( Result );

		lua_pop( L, 1 );
	}
}

void InputSinkEditor::save()
{
	lua_State		*L = luaL_newstate();

	lua_moo::luaNewState( L );

	QByteArray		 P = mEditor.text().join( "\n" ).toUtf8();

	int Error = luaL_loadbuffer( L, P, P.size(), mVerbName.toUtf8() );

	if( Error == 0 )
	{
		mVerb->setScript( P );

		mEditor.setQuit( true );
	}
	else
	{
		QString		Result = lua_tostring( L, -1 );

		mEditor.setStatusMessage( Result );

		lua_pop( L, 1 );
	}
}
