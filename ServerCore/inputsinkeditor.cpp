#include "inputsinkeditor.h"

#include <QDebug>

#include "connection.h"
#include "object.h"
#include "verb.h"
#include "lua_moo.h"
#include <lua.hpp>

InputSinkEditor::InputSinkEditor( Connection *C, Object *O, Verb *V, QStringList pText )
	: mConnection( C ), mObject( O ), mVerb( V )
{
	mConnection->setLineMode( Connection::REALTIME );

	connect( &mEditor, SIGNAL(output(QString)), this, SLOT(output(QString)) );

	mEditor.setText( pText );

	mEditor.redraw();
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
