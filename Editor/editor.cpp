#include "editor.h"

#include <QDebug>

Editor::Editor( QObject *pParent )
	: QObject( pParent ), mWindowSize( 80, 24 ), mANSI( 0 ), mQuit( false )
{
	mText << QString();

	mCursorScreenPosition = QPoint( -1, -1 );
	mCursorTextPosition   = QPoint( 0, 0 );
	mTextPosition         = QPoint( 0, 0 );
}

void Editor::addControlSlot( quint8 pASCII, QObject *pObject, QString pSlot, QString pDesc )
{
	mSlotMap.insert( pASCII, QPair<QObject *,QString>( pObject, pSlot ) );

	mSlotDesc.insert( pASCII, pDesc );
}

void Editor::remControlSlot( quint8 pASCII )
{
	mSlotMap.remove( pASCII );

	mSlotDesc.remove( pASCII );
}

void Editor::setText( QStringList pText )
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

//	emit output( QString( "W=%1 H=%2\n" ).arg( w ).arg( h ) );
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

	emit output( QString( "\x1b[%1;%2H" ).arg( y + 1 ).arg( x + 1 ) );

	mCursorScreenPosition = QPoint( x, y );
}

void Editor::setCursorScreenPosition( const QPoint &pP )
{
	setCursorScreenPosition( pP.x(), pP.y() );
}

void Editor::redraw()
{
	clearScreen();

	drawText();

	drawInfo();

	drawStatusMessage();

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
		else if( ch < 0x20 || ch >= 0x7f )
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
//			emit output( QString( "ANSI=%1" ).arg( pC ) );
			break;
	}

	updateCursorScreenPosition();

	mANSI = 0;

	mANSIArg.clear();
	mANSIArgs.clear();
}

void Editor::processCTRL( QChar pC )
{
	switch( pC.unicode() )
	{
		case 0x7f:	// DELETE
			if( !mCursorTextPosition.x() )
			{
				if( !mCursorTextPosition.y() )
				{
					return;
				}

				QString		TmpTxt = mText.takeAt( mCursorTextPosition.y() );
				QString		PrvTxt = mText.at( mCursorTextPosition.y() - 1 );

				mCursorTextPosition.ry()--;
				mCursorTextPosition.rx() = PrvTxt.size();

				PrvTxt.append( TmpTxt );

				mText[ mCursorTextPosition.y() ] = PrvTxt;

				updateCursorScreenPosition();

				drawText();
			}
			else
			{
				QString		TmpTxt = mText.at( mCursorTextPosition.y() );

				TmpTxt.remove( mCursorTextPosition.x() - 1, 1 );

				mText[ mCursorTextPosition.y() ] = TmpTxt;

				mCursorTextPosition.rx()--;

				updateCursorScreenPosition();

				drawText();
			}
			break;

		case 0x04:	// Ctrl + D - Delete the character under the cursor
			{
				QString		TmpTxt = mText.at( mCursorTextPosition.y() );

				if( mCursorTextPosition.x() == TmpTxt.size() )
				{
					if( mCursorTextPosition.y() + 1 >= mText.size() )
					{
						return;
					}

					QString	NxtTxt = mText.takeAt( mCursorTextPosition.y() + 1 );

					TmpTxt.append( NxtTxt );

					mText[ mCursorTextPosition.y() ] = TmpTxt;

					drawText();
				}
				else
				{
					TmpTxt.remove( mCursorTextPosition.x(), 1 );

					mText[ mCursorTextPosition.y() ] = TmpTxt;

					drawText();
				}
			}
			break;

		case 0x09:			// Tab
			processTEXT( pC );
			break;

		case 0x0d:			// Line Feed
			break;

		case 0x0a:			// Return
		case 0x00:
			if( mCursorTextPosition.x() == mText.at( mCursorTextPosition.y() ).size() )
			{
				mCursorTextPosition.rx() = 0;
				mCursorTextPosition.ry()++;

				mText.insert( mCursorTextPosition.y(), QString() );
			}
			else
			{
				QString		TmpTxt = mText.at( mCursorTextPosition.y() );
				QString		RemTxt = TmpTxt.mid( 0, mCursorTextPosition.x() );
				QString		MovTxt = TmpTxt.mid( mCursorTextPosition.x() );

				mText[ mCursorTextPosition.y() ] = RemTxt;

				mCursorTextPosition.rx() = 0;
				mCursorTextPosition.ry()++;

				mText.insert( mCursorTextPosition.y(), MovTxt );
			}

			updateCursorScreenPosition();

			drawText();
			break;

		case 0x18:			// ^X
			mQuit = true;
			emit quit();
			break;

		case 0x1b:			// ESC
			break;

		default:
			if( mSlotMap.contains( pC.toLatin1() ) )
			{
				QPair<QObject *,QString>	ObjSlt = mSlotMap.value( pC.toLatin1() );

				QMetaObject::invokeMethod( ObjSlt.first, ObjSlt.second.toLatin1().constData() );
			}
			else
			{
				qDebug() << QString::number( pC.unicode(), 16 );
			}
			break;
	}
}

void Editor::processTEXT( QChar pC )
{
	QString			CurTxt = mText.at( mCursorTextPosition.y() );

	if( mCursorTextPosition.x() == CurTxt.size() )
	{
		CurTxt.append( pC );

		emit output( pC );
	}
	else
	{
		CurTxt.insert( mCursorTextPosition.x(), pC );

		emit output( CurTxt.mid( mCursorTextPosition.x() ) );
	}

	mCursorTextPosition.rx()++;

	mText.replace( mCursorTextPosition.y(), CurTxt );

	updateCursorScreenPosition();
}

void Editor::clearScreen()
{
	setCursorScreenPosition( 0, 0 );

	emit output( "\x1b[2J" );		// clear whole screen
}

void Editor::setStatusMessage(QString pStatusMessage)
{
	mStatusMessage = pStatusMessage;

	drawStatusMessage();
}

void Editor::drawInfo()
{
	setCursorScreenPosition( 0, mWindowSize.height() - 2 );

	QStringList		InfoList;

	InfoList << QString( "Line: %1" ).arg( mCursorTextPosition.y() + 1 );

	for( QString S : mSlotDesc.values() )
	{
		InfoList << S;
	}

	InfoList << "Ctrl+X: Exit (without saving)";

	QString			InfoText = InfoList.join( " - " );

	if( InfoText.size() > mWindowSize.width() )
	{
		InfoText.resize( mWindowSize.width() );
	}

	InfoText.prepend( "\x1b[30;103m" );	// set black fg, bright yellow bg
	InfoText.append( "\x1b[0K\x1b[0m" );	// clear to EOL, reset attributes

	emit output( InfoText );

	setCursorScreenPosition( 0, 0 );
}

void Editor::drawStatusMessage()
{
	QPoint		TmpCurPos = mCursorScreenPosition;

	setCursorScreenPosition( 0, mWindowSize.height() - 1 );

	QString		TmpTxt = mStatusMessage.mid( 0, mWindowSize.width() );

	TmpTxt.prepend( "\x1b[0K" );

	emit output( TmpTxt );

	setCursorScreenPosition( TmpCurPos );
}

void Editor::updateCursorScreenPosition()
{
	QPoint		NewScrPos = mCursorTextPosition - mTextPosition;

	if( NewScrPos.y() >= mWindowSize.height() - 2 )
	{
		while( NewScrPos.y() >= mWindowSize.height() - 2 )
		{
			mTextPosition.ry() += 5;

			NewScrPos = mCursorTextPosition - mTextPosition;;
		}

		drawText();
	}

	if( NewScrPos.y() < 0 )
	{
		while( NewScrPos.y() < 0 )
		{
			mTextPosition.ry() = qMax( 0, mTextPosition.y() - 5 );

			NewScrPos = mCursorTextPosition - mTextPosition;;
		}

		drawText();
	}

	if( NewScrPos.x() >= mWindowSize.width() )
	{
		while( NewScrPos.x() >= mWindowSize.width() )
		{
			mTextPosition.rx() += 5;

			NewScrPos = mCursorTextPosition - mTextPosition;;
		}

		drawText();
	}

	if( NewScrPos.x() < 0 )
	{
		while( NewScrPos.x() < 0 )
		{
			mTextPosition.rx() = qMax( 0, mTextPosition.x() - 5 );

			NewScrPos = mCursorTextPosition - mTextPosition;;
		}

		drawText();
	}

	drawInfo();

	setCursorScreenPosition( NewScrPos );
}

void Editor::drawText( int pStart )
{
	QPoint		TmpCurPos = mCursorScreenPosition;

	for( int i = pStart ; i < mWindowSize.height() - 2 ; i++ )
	{
		setCursorScreenPosition( 0, i );

		QString		TmpTxt;

		if( i + mTextPosition.y() < mText.size() )
		{
			TmpTxt = mText.at( i + mTextPosition.y() );

			TmpTxt = TmpTxt.mid( mTextPosition.x(), mWindowSize.width() );
		}

		TmpTxt.prepend( "\x1b[0K" );

		emit output( TmpTxt );
	}

	setCursorScreenPosition( TmpCurPos );
}
