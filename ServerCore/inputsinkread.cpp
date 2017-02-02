#include "inputsinkread.h"

#include "connection.h"
#include "object.h"
#include "verb.h"
#include "mooapp.h"
#include "lua_moo.h"
#include "lua_task.h"
#include "objectmanager.h"

#include <lua.hpp>

InputSinkRead::InputSinkRead( Connection *C, ObjectId pObjectId, QString pVerbName )
	: mConnection( C ), mObjectId( pObjectId ), mVerbName( pVerbName )
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
			Object		*O = ObjectManager::o( mObjectId );
			Verb		*V = ( O ? O->verb( mVerbName ) : nullptr );

			if( V )
			{
				lua_task	 L( mConnection->id(), Task() );

				lua_pushlstring( L.L(), pData.toLatin1().constData(), pData.size() );

				lua_task::luaSetTask( L.L(), &L );

				L.setProgrammer( V->owner() );

				L.verbCall( mObjectId, V, 1 );
			}
		}

		return( false );
	}
	else
	{
		mInput.append( pData );
	}

	return( true );
}
