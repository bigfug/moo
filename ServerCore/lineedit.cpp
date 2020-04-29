#include "lineedit.h"

LineEdit::LineEdit( QObject *pParent )
	: QObject( pParent ), mLastChar( 0 ), mEditLinePosition( 10 ), mTerminalWidth( 80 ), mSecretChar( 0 ),
	  mAnsiEsc( 0 ), mAnsiPos( 0 )
{
	mPrompt = "> ";
}

void LineEdit::dataInput( const QByteArray &pData )
{
	for( QChar ch : pData )
	{
		QByteArray	Output;

		if( ch == '\n' || ch == '\r' )
		{
			emit lineOutput( mLineBuffer );

			mLineBuffer.clear();

			mAnsiPos = 0;
		}
		else if( mAnsiEsc == 1 )
		{
			if( ch == '[' )
			{
				mAnsiEsc++;

				mAnsiSeq.append( ch );
			}
			else
			{
				mLineBuffer.append( 0x1B );
				mLineBuffer.append( ch );

				mAnsiEsc = 0;
			}
		}
		else if( mAnsiEsc == 2 )
		{
			mAnsiSeq.append( ch );

			if( ch >= 64 && ch <= 126 )
			{
				if( mSecretChar.isNull() )
				{
					processAnsiSequence( mAnsiSeq, Output );
				}

				mAnsiEsc = 0;
			}
		}
		else
		{
			switch( ch.toLatin1() )
			{
				case 0x01:	// Ctrl + A - Cursor to start of line
					processCursorToStartOfLine( Output );
					break;

				case 0x02:	// Ctrl + B - Cursor to the left
					processCursorLeft( Output );
					break;

				case 0x04:	// Ctrl + D - Delete the character under the cursor
					processDelete( Output );
					break;

				case 0x05:	// Ctrl + E - Cursor to end of line
					processCursorToEndOfLine( Output );
					break;

				case 0x06:	// Ctrl + F - Cursor to the right
					processCursorRight( Output );
					break;

				case 0x08:	// BACKSPACE
					processBackspace( Output );
					break;

				case 0x09:
					break;

				case 0x0e:	// SHIFT OUT
				case 0x0f:	// SHIFT IN
					break;

				case 0x1b:	// ESCAPE
					mAnsiSeq.clear();
					mAnsiSeq.append( ch );
					mAnsiEsc++;
					break;

				case 0x7f:	// DELETE
					processBackspace( Output );
					break;

				default:
					if( ch >= 0x20 && ch < 0x7f )
					{
						processCharacter( ch, Output );
					}
					break;
			}
		}

		if( !Output.isEmpty() )
		{
			write( Output );
		}
	}
}

void LineEdit::setEditLinePosition( int y )
{
	if( mEditLinePosition != y )
	{
		mEditLinePosition = y;

		redraw();
	}
}

void LineEdit::setTerminalWidth( int w )
{
	if( mTerminalWidth != w )
	{
		mTerminalWidth = w;

		redraw();
	}
}

void LineEdit::redraw()
{
	QByteArray		A;

	//A.append( QString( "\x1b[%1;1H\x1b[K" ).arg( mEditLinePosition + 1 ).toLatin1() );

	A.append( "\x1b[K" );

	A.append( mPrompt );
	A.append( mLineBuffer );

	A.append( QString( "\x1b[%1G" ).arg( 1 + mAnsiPos + mPrompt.length() ).toLatin1() );

	write( A );
}

void LineEdit::processAnsiSequence( const QByteArray &pData, QByteArray &pOutput )
{
	QByteArray	CodeData = pData.mid( 2, pData.size() - 3 );
	quint8		CodeChar = pData.right( 1 ).at( 0 );

	switch( CodeChar )
	{
		case 'C':	// CURSOR FORWARD
			processCursorRight( pOutput );
			break;

		case 'D':	// CURSOR BACK
			processCursorLeft( pOutput );
			break;

		case '~':
			processFunctionKey( CodeData.toInt(), pOutput );
			break;

	}
}

void LineEdit::processFunctionKey( int pAnsiCode, QByteArray &pOutput )
{
	Qt::Key		K = Qt::Key_unknown;

	switch( pAnsiCode )
	{
		case 1: K = Qt::Key_Home; break;
		case 2: K = Qt::Key_Insert; break;
		case 3: K = Qt::Key_Delete; processDelete( pOutput ); break;
		case 4: K = Qt::Key_End; break;
		case 5: K = Qt::Key_PageUp; break;
		case 6: K = Qt::Key_PageDown; break;
	}
}

void LineEdit::processCharacter( QChar ch, QByteArray &pOutput )
{
	mLineBuffer.insert( mAnsiPos++, ch );

	if( true ) // echo() )
	{
		if( mAnsiPos < mLineBuffer.size() )
		{
			pOutput.append( mLineBuffer.mid( mAnsiPos - 1 ).append( QString( "\x1b[%1D" ).arg( mLineBuffer.size() - mAnsiPos ) ) );
		}
		else if( !mSecretChar.isNull() )
		{
			pOutput.append( mSecretChar );
		}
		else
		{
			pOutput.append( ch );
		}
	}
}

void LineEdit::processCursorLeft( QByteArray &pOutput )
{
	if( mAnsiPos > 0 )
	{
		mAnsiPos--;

		pOutput.append( "\x1b[D" );
	}
}

void LineEdit::processCursorRight( QByteArray &pOutput )
{
	if( mAnsiPos < mLineBuffer.size() )
	{
		mAnsiPos++;

		pOutput.append( "\x1b[C" );
	}
}

void LineEdit::processCursorToStartOfLine( QByteArray &pOutput )
{
	if( mAnsiPos > 0 )
	{
		mAnsiPos = 0;

		pOutput.append( QString( "\x1b[%1G" ).arg( 1 + mAnsiPos + mPrompt.length() ).toLatin1() );
	}
}

void LineEdit::processCursorToEndOfLine( QByteArray &pOutput )
{
	if( mAnsiPos < mLineBuffer.size() )
	{
		mAnsiPos = mLineBuffer.size();

		pOutput.append( QString( "\x1b[%1G" ).arg( 1 + mAnsiPos + mPrompt.length() ).toLatin1() );
	}
}

void LineEdit::processDelete( QByteArray &pOutput )
{
	if( mAnsiPos < mLineBuffer.size() )
	{
		mLineBuffer.remove( mAnsiPos, 1 );

		if( mSecretChar.isNull() ) //echo() )
		{
			pOutput.append( mLineBuffer.mid( mAnsiPos ) );
			pOutput.append( QString( " \x1b[%1D" ).arg( mLineBuffer.size() + 1 - mAnsiPos ) );
		}
	}
}

void LineEdit::processBackspace( QByteArray &pOutput )
{
	if( mAnsiPos > 0 )
	{
		mLineBuffer.remove( --mAnsiPos, 1 );

		if( mSecretChar.isNull() )  // echo() )
		{
			pOutput.append( "\x1b[D" );
			pOutput.append( mLineBuffer.mid( mAnsiPos ) );
			pOutput.append( QString( " \x1b[%1D" ).arg( mLineBuffer.size() + 1 - mAnsiPos ) );
		}
		else
		{
			pOutput.append( "\x1b[D \x1b[D" );
		}
	}
}

void LineEdit::write( const char *pData )
{
	emit dataOutput( QByteArray( pData ) );
}

void LineEdit::write( const QString &pData )
{
	emit dataOutput( pData.toLatin1() );
}

void LineEdit::write( const QByteArray &pData )
{
	emit dataOutput( pData );
}
