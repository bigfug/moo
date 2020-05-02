#include "lua_utilities.h"
#include "lua_object.h"
#include "lua_task.h"
#include "objectmanager.h"

#include "verb.h"
#include "property.h"

#include "lua_prop.h"

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

		case QVariant::List:
			{
				const QVariantList		&VarLst = pV.toList();

				lua_newtable( L );

				for( int i = 0 ; i < VarLst.size() ; i++ )
				{
					lua_pushinteger( L, i + 1 );

					luaL_pushvariant( L, VarLst.at( i ) );

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


QVariant lua_util::stringToObject( QVariant V )
{
	if( V.type() != QVariant::String )
	{
		return( V );
	}

	QString					ST = V.toString();

	if( ST.startsWith( "##" ) )
	{
		ST.remove( 0, 1 );

		return( ST );
	}

	if( ST.startsWith( '#' ) )
	{
		ST.remove( 0, 1 );

		lua_object::luaHandle	LH;

		LH.O = ST.toInt();

		return( QVariant::fromValue( LH ) );
	}

	return( V );
}

void lua_util::stringsToObjects( QVariantMap &PrpDat )
{
	for( QVariantMap::iterator it = PrpDat.begin() ; it != PrpDat.end() ; it++ )
	{
		if( it.value().type() == QVariant::String )
		{
			it.value() = stringToObject( it.value() );
		}
		else if( it.value().type() == QVariant::Map )
		{
			QVariantMap		VM = it.value().toMap();

			stringsToObjects( VM );

			it.value() = VM;
		}
	}
}

void lua_util::objectsToStrings( QVariantMap &PrpDat )
{
	for( QVariantMap::iterator it = PrpDat.begin() ; it != PrpDat.end() ; it++ )
	{
		if( it.value().canConvert<lua_object::luaHandle>() )
		{
			lua_object::luaHandle	LH = it.value().value<lua_object::luaHandle>();

			it.value() = QString( "#%1" ).arg( LH.O );
		}
		else if( it.value().type() == QVariant::String )
		{
			QString					ST = it.value().toString();

			if( ST.startsWith( '#' ) )
			{
				ST.prepend( '#' );

				it.value() = ST;
			}
		}
		else if( it.value().type() == QVariant::Map )
		{
			QVariantMap		VM = it.value().toMap();

			objectsToStrings( VM );

			it.value() = VM;
		}
	}
}

QVariant lua_util::lua_tovariant( lua_State *L, const Property *pRefPrp, int pIndex )
{
	QVariant		V;

	if( !strcmp( pRefPrp->value().typeName(), lua_object::luaHandle::mTypeName ) )
	{
		lua_object::luaHandle		 H;

		H.O = lua_object::argId( L, pIndex );

		V.setValue( H );
	}
	else
	{
		switch( pRefPrp->type() )
		{
			case QVariant::Bool:
				{
					luaL_checktype( L, pIndex, LUA_TBOOLEAN );

					bool	v = lua_toboolean( L, pIndex );

					V.setValue( v );
				}
				break;

			case QVariant::Double:
				{
					lua_Number	v = luaL_checknumber( L, pIndex );

					V.setValue( v );
				}
				break;

			case QVariant::String:
				{
					size_t		l;

					const char *v = luaL_checklstring( L, pIndex, &l );

					V.setValue( QString::fromLatin1( v, l ) );
				}
				break;

			case QVariant::Map:
				{
					luaL_checktype( L, pIndex, LUA_TTABLE );

					lua_prop::luaNewRecurse( L, pIndex, V );
				}
				break;

			default:
				throw mooException( E_TYPE, QString( "Unknown property value type: %1" ).arg( pRefPrp->type() ) );
		}
	}

	return( V );
}
