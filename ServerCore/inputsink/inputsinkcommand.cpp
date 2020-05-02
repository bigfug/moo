#include "inputsinkcommand.h"

#include <QDebug>

InputSinkCommand::InputSinkCommand( Connection *C )
	: mConnection( C )
{
	connect( &mLineEdit, &LineEdit::commandHistoryLookup, [=]( int pIndex )
	{
		mLineEdit.setCommandHistoryEntry( mCommandHistory.at( pIndex ).toUtf8() );
	} );

	connect( &mLineEdit, &LineEdit::lineOutput, [=]( const QByteArray &pLine )
	{
		if( !mCommandHistory.isEmpty() )
		{
			if( pLine != mCommandHistory.last() )
			{
				mCommandHistory << pLine;
			}
		}
		else
		{
			mCommandHistory << pLine;
		}

		while( mCommandHistory.size() > 50 )
		{
			mCommandHistory.takeFirst();
		}

		mLineEdit.setCommandHistoryCount( mCommandHistory.size() );

		mConnection->write( "\r\n" );

		mConnection->performTask( pLine );
	} );

	connect( &mLineEdit, &LineEdit::dataOutput, [=]( const QByteArray &pData )
	{
		mConnection->write( pData );
	} );
}

bool InputSinkCommand::input( const QString &pData )
{
//	qDebug() << "InputSinkCommand::input" << pData;

	mLineEdit.dataInput( pData.toLatin1() );

	return( true );
}

// output() receives data from Connection::notify()
// has the option to block printing (returns true)
// or allows it (returns false)

bool InputSinkCommand::output( const QString &pData )
{
	mConnection->write( "\x1b[1G\x1b[K" );	// cursor to column 0 and clear to end of line

	mConnection->write( pData );

	mConnection->write( "\r\n" );

	mLineEdit.redraw();

	return( true );
}
