#include "inputsinkedittext.h"

#include "connection.h"
#include "object.h"
#include "verb.h"
#include "lua_moo.h"
#include "objectmanager.h"

#include <lua.hpp>

InputSinkEditText::InputSinkEditText( Connection *C, Object *O, Property *P, QStringList pText )
	: mConnection( C ), mObject( O ), mProperty( P )
{
	mConnection->setLineMode( Connection::REALTIME );

	mEditor.setSize( mConnection->terminalSize() );

	connect( &mEditor, SIGNAL(output(QString)), this, SLOT(output(QString)) );

	mEditor.setText( pText );

	mEditor.redraw();

	mEditor.addControlSlot( 19, this, "save" );
}

bool InputSinkEditText::input( const QString &pData )
{
	mEditor.input( pData );

	if( mEditor.hasQuit() )
	{
		mConnection->notify( "\x1b[2J\x1b[1;1H" );

		mConnection->setLineMode( Connection::EDIT );
	}

	return( !mEditor.hasQuit() );
}

void InputSinkEditText::output( const QString &pData )
{
	mConnection->notify( pData );
}

void InputSinkEditText::save()
{
	const QString		Text = mEditor.text().join( "\n" );

	if( Text != mProperty->value() )
	{
		mProperty->setValue( Text );
	}

	mEditor.setQuit( true );
}
