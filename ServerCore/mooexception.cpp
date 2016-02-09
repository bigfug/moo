#include "mooexception.h"

void mooException::lua_pushexception(lua_State *L)
{
	qDebug() << mMessage;

	luaL_where( L, 1 );
	lua_pushstring( L, mMessage.toLatin1() );
	lua_concat( L, 2 );
}
