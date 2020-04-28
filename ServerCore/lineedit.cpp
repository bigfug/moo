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
		QByteArray	Tmp;

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
					processAnsiSequence( mAnsiSeq, Tmp );
				}

				mAnsiEsc = 0;
			}
		}
		else
		{
			switch( ch.toLatin1() )
			{
				case 0x08:	// BACKSPACE
					if( mAnsiPos > 0 )
					{
						mLineBuffer.remove( --mAnsiPos, 1 );

						if( mSecretChar.isNull() )  // echo() )
						{
							Tmp.append( "\x1b[D" );
							Tmp.append( mLineBuffer.mid( mAnsiPos ) );
							Tmp.append( QString( " \x1b[%1D" ).arg( mLineBuffer.size() + 1 - mAnsiPos ) );
						}
						else
						{
							Tmp.append( "\x1b[D \x1b[D" );
						}
					}
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
					processDelete( Tmp );
					break;

				default:
					if( ch >= 0x20 && ch < 0x7f )
					{
						mLineBuffer.insert( mAnsiPos++, ch );

						if( true ) // echo() )
						{
							if( mAnsiPos < mLineBuffer.size() )
							{
								Tmp.append( mLineBuffer.mid( mAnsiPos - 1 ).append( QString( "\x1b[%1D" ).arg( mLineBuffer.size() - mAnsiPos ) ) );
							}
							else if( !mSecretChar.isNull() )
							{
								Tmp.append( mSecretChar );
							}
							else
							{
								Tmp.append( ch );
							}
						}
					}
					break;
			}
		}

		if( !Tmp.isEmpty() )
		{
			write( Tmp );
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

void LineEdit::processAnsiSequence( const QByteArray &pData, QByteArray &pTemp )
{
	QByteArray	CodeData = pData.mid( 2, pData.size() - 3 );
	quint8		CodeChar = pData.right( 1 ).at( 0 );

	switch( CodeChar )
	{
		case 'C':	// CURSOR FORWARD
			processCursorRight( pTemp );
			break;

		case 'D':	// CURSOR BACK
			processCursorLeft( pTemp );
			break;

		case '~':
			processFunctionKey( CodeData.toInt(), pTemp );
			break;

	}
}

void LineEdit::processFunctionKey( int pAnsiCode, QByteArray &pTemp )
{
	Qt::Key		K = Qt::Key_unknown;

	switch( pAnsiCode )
	{
		case 1: K = Qt::Key_Home; break;
		case 2: K = Qt::Key_Insert; break;
		case 3: K = Qt::Key_Delete; processDelete( pTemp ); break;
		case 4: K = Qt::Key_End; break;
		case 5: K = Qt::Key_PageUp; break;
		case 6: K = Qt::Key_PageDown; break;
	}
}

void LineEdit::processCursorLeft( QByteArray &pTemp )
{
	if( mAnsiPos > 0 )
	{
		mAnsiPos--;

		pTemp.append( "\x1b[D" );
	}
}

void LineEdit::processCursorRight( QByteArray &pTemp )
{
	if( mAnsiPos < mLineBuffer.size() )
	{
		mAnsiPos++;

		pTemp.append( "\x1b[C" );
	}
}

void LineEdit::processDelete( QByteArray &pTemp )
{
	if( mAnsiPos < mLineBuffer.size() )
	{
		mLineBuffer.remove( mAnsiPos, 1 );

		if( mSecretChar.isNull() ) //echo() )
		{
			pTemp.append( mLineBuffer.mid( mAnsiPos ) );
			pTemp.append( QString( " \x1b[%1D" ).arg( mLineBuffer.size() + 1 - mAnsiPos ) );
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
