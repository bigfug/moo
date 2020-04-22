#include "lua_osc.h"
#include "lua_moo.h"

#include "lua_task.h"
#include "osc.h"

const luaL_Reg lua_osc::mLuaStatic[] =
{
	{ "oscsend", lua_osc::luaSend },
	{ 0, 0 }
};

void lua_osc::initialise()
{
	lua_moo::addGet( mLuaStatic );
}

void lua_osc::luaRegisterState( lua_State *L )
{
	Q_UNUSED( L )
}

int lua_osc::luaSend( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	try
	{
		if( !Command->isWizard() )
		{
			throw( mooException( E_NACC, "programmer is not wizard" ) );
		}

		luaL_checkstring( L, 1 );

		const int			 argc = lua_gettop( L );

		QString		Path = QString( lua_tostring( L, 1 ) );

		QVariant	Args;

		if( argc >= 2 )
		{
			if( lua_isnumber( L, 2 ) )
			{
				Args = lua_tonumber( L, 2 );
			}
			else if( lua_isstring( L, 2 ) )
			{
				Args = QString( lua_tostring( L, 2 ) );
			}
			else if( lua_isboolean( L, 2 ) )
			{
				Args = lua_toboolean( L, 2 );
			}
		}

		OSC	*DEV = OSC::devices().isEmpty() ? nullptr : OSC::devices().first();

		if( DEV )
		{
			DEV->sendData( Path, Args );
		}
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( const std::exception &e )
	{
		Command->setException( mooException( E_EXCEPTION, e.what() ) );
	}

	return( Command->lua_pushexception() );
}
