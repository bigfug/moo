#include "editor.h"

#include <QDebug>

Editor::Editor( QObject *pParent )
	: QObject( pParent ), mWindowSize( 80, 24 ), mANSI( 0 ), mQuit( false )
{
	mText << QString();

	mCursorScreenPosition = QPoint( -1, -1 );
	mCursorTextPosition   = QPoint( 0, 0 );
}

void Editor::setText(QStringList pText)
{
	mText = pText;
}

void Editor::setSize( int w, int h )
{
	if( mWindowSize.width() == w && mWindowSize.height() == h )
	{
		return;
	}

	mWindowSize.setWidth( w );
	mWindowSize.setHeight( h );

	emit sizeChanged( w, h );

	emit output( QString( "W=%1 H=%2\n" ).arg( w ).arg( h ) );
}

void Editor::setSize( const QSize &pSize )
{
	setSize( pSize.width(), pSize.height() );
}

void Editor::setCursorScreenPosition( int x, int y )
{
	if( mCursorScreenPosition.x() == x && mCursorScreenPosition.y() == y )
	{
		return;
	}

	qDebug() << QString( "\033[%1;%2H" ).arg( y + 1 ).arg( x + 1 );

	emit output( QString( "\033[%1;%2H" ).arg( y + 1 ).arg( x + 1 ) );

	mCursorScreenPosition = QPoint( x, y );
}

void Editor::redraw()
{
	clear();

	for( int i = 0 ; i < qMin( mWindowSize.height(), mText.size() ); i++ )
	{
		setCursorScreenPosition( 0, i );

		emit output( mText.at( i ) );
	}

	drawInfo();

	setCursorScreenPosition( 0, 0 );
}

void Editor::input( const QString &pData )
{
	for( const QChar ch : pData )
	{
		if( ch == 0x1b )
		{
			if( mANSI == 0 )
			{
				mANSI = 1;

				continue;
			}

			mANSI = 0;
		}

		if( mANSI == 1 )
		{
			if( ch == '[' )
			{
				mANSI = 2;

				mANSIArg.clear();
				mANSIArgs.clear();

				continue;
			}

			processCTRL( 0x1b );

			mANSI = 0;
		}

		if( mANSI == 2 )
		{
			processANSI( ch );
		}
		else if( ch < 0x20 )
		{
			processCTRL( ch );
		}
		else
		{
			processTEXT( ch );
		}
	}
}

void Editor::processANSI( QChar pC )
{
	if( pC.isDigit() )
	{
		mANSIArg.append( pC );

		return;
	}

	if( pC == ';' )
	{
		if( !mANSIArg.isEmpty() )
		{
			mANSIArgs.append( mANSIArg );

			mANSIArg.clear();
		}

		return;
	}

	if( !mANSIArg.isEmpty() )
	{
		mANSIArgs.append( mANSIArg );

		mANSIArg.clear();
	}

	switch( pC.unicode() )
	{
		case 'A':	// Cursor Up
			if( mCursorTextPosition.y() > 0 )
			{
				mCursorTextPosition.ry()--;

				int		LinLen = mText.at( mCursorTextPosition.y() ).length();

				if( mCursorTextPosition.x() > LinLen )
				{
					mCursorTextPosition.rx() = LinLen;
				}
			}
			break;

		case 'B':	// Cursor Down
			if( mCursorTextPosition.y() < mText.size() - 1 )
			{
				mCursorTextPosition.ry()++;

				int		LinLen = mText.at( mCursorTextPosition.y() ).length();

				if( mCursorTextPosition.x() > LinLen )
				{
					mCursorTextPosition.rx() = LinLen;
				}
			}
			break;

		case 'C':	// Cursor Right
			if( mCursorTextPosition.x() < mText.at( mCursorTextPosition.y() ).length() )
			{
				mCursorTextPosition.rx()++;
			}
			break;

		case 'D':	// Cursor Left
			if( mCursorTextPosition.x() > 0 )
			{
				mCursorTextPosition.rx()--;
			}
			break;

		default:
			emit output( QString( "ANSI=%1" ).arg( pC ) );
			break;
	}

	mANSI = 0;

	mANSIArg.clear();
	mANSIArgs.clear();
}

void Editor::processCTRL( QChar pC )
{
	switch( pC.unicode() )
	{
		case 0x09:			// Tab
			processTEXT( pC );
			break;

		case 0x0a:			// Return
			mCursorTextPosition.rx() = 0;
			mCursorTextPosition.ry()++;
			mText.insert( mCursorTextPosition.y(), QString() );
			break;

		case 0x18:			// ^X
			mQuit = true;
			emit quit();
			break;

		case 0x1b:			// ESC
			break;

		default:
			emit output( QString::number( pC.unicode(), 16 ) );
			break;
	}
}

void Editor::processTEXT( QChar pC )
{
	QString			CurTxt = mText.at( mCursorTextPosition.y() );

	CurTxt.insert( mCursorTextPosition.x(), pC );

	mCursorTextPosition.rx()++;

	mText.replace( mCursorTextPosition.y(), CurTxt );

	emit output( pC );
}

void Editor::clear()
{
	setCursorScreenPosition( 0, 0 );

	emit output( "\e[2J" );
}

void Editor::drawInfo()
{
	setCursorScreenPosition( 0, mWindowSize.height() - 2 );

	emit output( QString().fill( 'X', mWindowSize.width() - 1 ) );

	setCursorScreenPosition( 0, 0 );
}
