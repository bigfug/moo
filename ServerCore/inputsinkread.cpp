#include "inputsinkread.h"

#include "connection.h"
#include "object.h"
#include "verb.h"
#include "mooapp.h"
#include "lua_moo.h"
#include "lua_object.h"
#include "lua_task.h"
#include "objectmanager.h"
#include "connection.h"

#include <lua.hpp>

InputSinkRead::InputSinkRead( Connection *C, ObjectId pObjectId, QString pVerbName, QVariantMap pReadArgs, QVariantList pVerbArgs )
	: mConnection( C ), mObjectId( pObjectId ), mVerbName( pVerbName ), mReadArgs( pReadArgs ), mVerbArgs( pVerbArgs ),
	  mAnsiEsc( 0 ), mAnsiPos( 0 )
{
	if( mReadArgs.value( "password", false ).toBool() )
	{
		mConnection->setLineMode( Connection::REALTIME );
	}
}

bool InputSinkRead::input( const QString &pData )
{
	if( mConnection->lineMode() == Connection::REALTIME )
	{
		bool		EndRead = false;

		QChar		SecretChar = 0;

		if( mReadArgs.value( "password", false ).toBool() )
		{
			SecretChar = '*';
		}

		for( QChar ch : pData )
		{
			if( ch == '\n' || ch == '\r' )
			{
				EndRead = true;
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
					mInput.append( 0x1B );
					mInput.append( ch );

					mAnsiEsc = 0;
				}
			}
			else if( mAnsiEsc == 2 )
			{
				mAnsiSeq.append( ch );

				if( ch >= 64 && ch <= 126 )
				{
					if( SecretChar.isNull() )
					{
						processAnsiSequence( mAnsiSeq );
					}

					mAnsiEsc = 0;
				}
			}
			else
			{
				QByteArray	Tmp;

				switch( ch.toLatin1() )
				{
					case 0x08:	// BACKSPACE
						if( mAnsiPos > 0 )
						{
							mInput.remove( --mAnsiPos, 1 );

							if( SecretChar.isNull() )  // echo() )
							{
								Tmp.append( "\x1b[D" );
								Tmp.append( mInput.mid( mAnsiPos ) );
								Tmp.append( QString( " \x1b[%1D" ).arg( mInput.size() + 1 - mAnsiPos ) );
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
						if( mAnsiPos < mInput.size() )
						{
							mInput.remove( mAnsiPos, 1 );

							if( SecretChar.isNull() ) //echo() )
							{
								Tmp.append( mInput.mid( mAnsiPos ) );
								Tmp.append( QString( " \x1b[%1D" ).arg( mInput.size() + 1 - mAnsiPos ) );
							}
						}
						break;

					default:
						if( ch >= 0x20 && ch < 0x7f )
						{
							mInput.insert( mAnsiPos++, ch );

							if( true ) // echo() )
							{
								if( mAnsiPos < mInput.size() )
								{
									Tmp.append( mInput.mid( mAnsiPos - 1 ).append( QString( "\x1b[%1D" ).arg( mInput.size() - mAnsiPos ) ) );
								}
								else if( !SecretChar.isNull() )
								{
									Tmp.append( SecretChar );
								}
								else
								{
									Tmp.append( ch );
								}
							}
						}
						break;
				}

				if( !Tmp.isEmpty() )
				{
					mConnection->write( Tmp );
				}
			}
		}

		if( !EndRead )
		{
			return( true );
		}

		mConnection->write( "\r\n" );

		mConnection->setLineMode( Connection::EDIT );
	}
	else
	{
		mInput = pData;
	}

	if( true )
	{
		Object		*O = ObjectManager::o( mObjectId );
		Verb		*V = ( O ? O->verb( mVerbName ) : nullptr );

		if( V )
		{
			lua_task	 L( mConnection->id(), Task() );

			lua_task::luaSetTask( L.L(), &L );

			L.setProgrammer( V->owner() );

			int			ArgCnt = 1;

			lua_pushlstring( L.L(), mInput.toLatin1().constData(), mInput.size() );

			for( const QVariant &V : mVerbArgs )
			{
				switch( QMetaType::Type( V.type() ) )
				{
					case QMetaType::Bool:
						lua_pushboolean( L.L(), V.toBool() );
						ArgCnt++;
						break;

					case QMetaType::Double:
						lua_pushnumber( L.L(), V.toDouble() );
						ArgCnt++;
						break;

					case QMetaType::Float:
						lua_pushnumber( L.L(), V.toFloat() );
						ArgCnt++;
						break;

					case QMetaType::Int:
						lua_pushinteger( L.L(), V.toInt() );
						ArgCnt++;
						break;

					case QMetaType::QString:
						lua_pushstring( L.L(), V.toString().toLatin1().constData() );
						ArgCnt++;
						break;

					default:
						if( V.typeName() == QStringLiteral( "lua_object::luaHandle" ) )
						{
							lua_object::luaHandle	LH = V.value<lua_object::luaHandle>();

							lua_object::lua_pushobjectid( L.L(), LH.O );

							ArgCnt++;
						}
						break;
				}
			}

			L.verbCall( mObjectId, V, ArgCnt );
		}

		return( false );
	}

	return( true );
}

void InputSinkRead::processAnsiSequence( const QByteArray &pData )
{
	if( pData.size() == 3 )
	{
		switch( static_cast<quint8>( pData.at( 2 ) ) )
		{
			case 'C':	// CURSOR FORWARD
				if( mAnsiPos < mInput.size() )
				{
					mAnsiPos++;

					mConnection->write( "\x1b[C" );
				}
				break;

			case 'D':	// CURSOR BACK
				if( mAnsiPos > 0 )
				{
					mAnsiPos--;

					mConnection->write( "\x1b[D" );
				}
				break;

		}
	}
}

