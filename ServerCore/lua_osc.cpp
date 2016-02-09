#include "lua_osc.h"
#include "lua_moo.h"

const luaL_Reg lua_osc::mLuaStatic[] =
{
//	{ "connection", lua_osc::luaCon },
	{ 0, 0 }
};

void lua_osc::initialise()
{
	lua_moo::addGet( mLuaStatic );
}

void lua_osc::luaRegisterState( lua_State *L )
{
}
