#include "mooexception.h"

#include <QStringList>

#include "lua_task.h"

void mooException::lua_pushexception( lua_State * L ) const
{
	lua_task			*Command = lua_task::luaGetTask( L );
	QString				 VrbStr;

	luaL_where( L, 1 );

	if( Command )
	{
		QStringList		VrbLst = Command->taskVerbStack();

		if( !VrbLst.isEmpty() )
		{
			VrbStr = VrbLst.join( " > " ).append( ": " );
		}
	}

	lua_pushstring( L, VrbStr.toLatin1() );
	lua_pushstring( L, mMessage.toLatin1() );
	lua_concat( L, 3 );
}
