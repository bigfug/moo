#include "lua_utilities.h"
#include "lua_object.h"

#if LUA_VERSION_NUM < 502

void *luaL_testudata( lua_State *L, int ud, const char *tname )
{
	void *p = lua_touserdata(L, ud);

	if (p != NULL)
	{  /* value is a userdata? */
		if (lua_getmetatable(L, ud))
		{  /* does it have a metatable? */
			lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */

			if (lua_rawequal(L, -1, -2))
			{  /* does it have the correct mt? */
				lua_pop(L, 2);  /* remove both metatables */
				return p;
			}
		}
	}
	return NULL;  /* to avoid warnings */
}

#endif

int luaL_pushvariant( lua_State *L, const QVariant &pV )
{
	if( strcmp( pV.typeName(), "lua_object::luaHandle" ) == 0 )
	{
		lua_object::luaHandle		H = pV.value<lua_object::luaHandle>();

		lua_object::lua_pushobjectid( L, H.O );

		return( 1 );
	}

	switch( pV.type() )
	{
		case QVariant::String:
			lua_pushstring( L, pV.toString().toLatin1() );
			return( 1 );

		case QVariant::Bool:
			lua_pushboolean( L, pV.toBool() );
			return( 1 );

		case QVariant::Double:
			lua_pushnumber( L, pV.toDouble() );
			return( 1 );

		case QVariant::Int:
			lua_pushinteger( L, pV.toInt() );
			return( 1 );

		case QVariant::Map:
			{
				const QVariantMap		&VarMap = pV.toMap();

				lua_newtable( L );

				for( QVariantMap::const_iterator it = VarMap.constBegin() ; it != VarMap.constEnd() ; it++ )
				{
					QString		K = it.key();
					bool		B;
					int			V = K.toInt( &B );

					if( B )
					{
						lua_pushinteger( L, V );
					}
					else
					{
						lua_pushstring( L, K.toLatin1().constData() );
					}

					luaL_pushvariant( L, *it );
					lua_settable( L, -3 );
				}

				return( 1 );
			}

		default:
			lua_pushnil( L );
			break;
	}

	return( 1 );
}
