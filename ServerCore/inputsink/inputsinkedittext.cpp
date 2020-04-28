#include "inputsinkedittext.h"

#include "connection.h"
#include "object.h"
#include "verb.h"
#include "lua_moo.h"
#include "objectmanager.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

InputSinkEditText::InputSinkEditText(Connection *C, ObjectId pObjectId, QString pPropName, QStringList pText )
	: mConnection( C ), mObjectId( pObjectId ), mPropName( pPropName )
{
	mEditor.setSize( mConnection->terminalSize() );

	connect( &mEditor, &Editor::output, this, &InputSinkEditText::editorOutput );

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

		mConnection->redrawBuffer();
	}

	return( !mEditor.hasQuit() );
}

void InputSinkEditText::editorOutput( const QString &pData )
{
	mConnection->notify( pData );
}

void InputSinkEditText::save()
{
	const QString		Text = mEditor.text().join( "\n" );

	Object			*O = ObjectManager::o( mObjectId );
	Property		*P = ( O ? O->prop( mPropName ) : nullptr );

	if( P )
	{
		P->setValue( Text );
	}

	mEditor.setQuit( true );
}
