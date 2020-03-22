#include "lua_text.h"

#include "objectmanager.h"
#include "lua_moo.h"
#include "lua_object.h"
#include "connection.h"
#include "lua_utilities.h"
#include "mooexception.h"
#include "lua_task.h"
#include "connectionmanager.h"

const luaL_Reg lua_text::mLuaStatic[] =
{
	{ "s", lua_text::luaPronounSubstitution },
	{ "bold", lua_text::luaBold },
	{ 0, 0 }
};

void lua_text::initialise()
{
	lua_moo::addFunctions( mLuaStatic );
}

void lua_text::luaRegisterState( lua_State *L )
{
	Q_UNUSED( L )
}

int lua_text::luaPronounSubstitution( lua_State *L )
{
	bool				 LuaErr = false;

	try
	{
		lua_task			*Command = lua_task::luaGetTask( L );
		const char			*S = luaL_checkstring( L, 1 );

		QString				 D;
		bool				 E = false;
		QString				 SubStr;

		Object				*O = nullptr;
		Object				*Player = nullptr;
		Object				*Location = nullptr;
		Object				*Direct = nullptr;
		Object				*Indirect = nullptr;

		while( *S )
		{
			const char		 c = *S++;

			if( E )
			{
				if( SubStr.isEmpty() )
				{
					SubStr += c;

					if( c != '(' && c != '[' )
					{
						E = false;
					}
				}
				else if( SubStr.startsWith( '(' ) )
				{
					SubStr += c;

					if( c == ')' )
					{
						E = false;
					}
				}
				else if( SubStr.startsWith( '[' ) )
				{
					SubStr += c;

					if( c == ']' )
					{
						E = false;
					}
				}

				if( !E && !SubStr.isEmpty() )
				{
					QString				 SubRes;

					if( SubStr.size() == 1 )
					{
						QString				 SubLwr = SubStr.toLower();

						if( SubLwr == QStringLiteral( "n" ) )
						{
							if( ( Player = ( Player ? Player : ObjectManager::o( Command->task().player() ) ) ) )
							{
								SubRes = Player->name();
							}
						}
						else if( SubLwr == QStringLiteral( "o" ) )
						{
							if( ( O = ( O ? O : ObjectManager::o( Command->task().object() ) ) ) )
							{
								SubRes = O->name();
							}
						}
						else if( SubLwr == QStringLiteral( "d" ) )
						{
							if( Command->task().directObjectId() != OBJECT_NONE )
							{
								if( ( Direct = ( Direct ? Direct : ObjectManager::o( Command->task().directObjectId() ) ) ) )
								{
									SubRes = Direct->name();
								}
							}
							else
							{
								SubRes = Command->task().directObjectName();
							}
						}
						else if( SubLwr == QStringLiteral( "i" ) )
						{
							if( Command->task().indirectObjectId() != OBJECT_NONE )
							{
								if( ( Indirect = ( Indirect ? Indirect : ObjectManager::o( Command->task().indirectObjectId() ) ) ) )
								{
									SubRes = Indirect->name();
								}
							}
							else
							{
								SubRes = Command->task().indirectObjectName();
							}
						}
						else if( SubLwr == QStringLiteral( "l" ) )
						{
							if( !Location )
							{
								if( ( Player = ( Player ? Player : ObjectManager::o( Command->task().player() ) ) ) )
								{
									Location = ObjectManager::o( Player->location() );
								}
							}

							if( Location )
							{
								SubRes = Location->name();
							}
						}
						else
						{
							D += "%" + SubStr;
						}
					}
					else if( SubStr.startsWith( '[' ) && SubStr.endsWith( ']' ) )
					{

					}
					else if( SubStr.startsWith( '(' ) && SubStr.endsWith( ')' ) )
					{

					}
					else
					{
						D += "%" + SubStr;
					}

					D += SubRes;

					SubStr.clear();
				}
			}
			else if( c == '%' )
			{
				if( E )
				{
					D += '%';
				}
				else
				{
					E = true;
				}
			}
			else
			{
				D += c;
			}
		}

		if( !SubStr.isEmpty() )
		{
			D += "%" + SubStr;
		}

		lua_pushstring( L, D.toLatin1().constData() );

		return( 1 );
	}
	catch( mooException e )
	{
		e.lua_pushexception( L );

		LuaErr = true;
	}
	catch( ... )
	{

	}

	return( LuaErr ? lua_error( L ) : 0 );
}

int lua_text::luaBold( lua_State *L )
{
	const char			*S = luaL_checkstring( L, 1 );

	lua_pushstring( L, QString( "*%1*" ).arg( S ).toLatin1().constData() );

	return( 1 );
}

