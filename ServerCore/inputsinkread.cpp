#include "inputsinkread.h"

#include "connection.h"
#include "object.h"
#include "verb.h"
#include "mooapp.h"
#include "lua_moo.h"
#include "lua_object.h"
#include "lua_task.h"
#include "objectmanager.h"

#include <lua.hpp>

InputSinkRead::InputSinkRead( Connection *C, ObjectId pObjectId, QString pVerbName, QVariantMap pReadArgs, QVariantList pVerbArgs )
	: mConnection( C ), mObjectId( pObjectId ), mVerbName( pVerbName ), mReadArgs( pReadArgs ), mVerbArgs( pVerbArgs )
{
}

bool InputSinkRead::input( const QString &pData )
{
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

			lua_pushlstring( L.L(), pData.toLatin1().constData(), pData.size() );

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
	else
	{
		mInput.append( pData );
	}

	return( true );
}
