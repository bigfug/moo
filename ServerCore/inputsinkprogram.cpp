#include "inputsinkprogram.h"

#include "connection.h"
#include "object.h"
#include "verb.h"
#include "lua_moo.h"
#include "objectmanager.h"

#include <lua.hpp>

InputSinkProgram::InputSinkProgram( Connection *C, Object *O, Verb *V, const QString &pVerbName )
{
	mConnection = C;
	mObject = O;
	mVerb   = V;
	mVerbName = pVerbName;
}

bool InputSinkProgram::input( const QString &pData )
{
	if( pData.compare( "." ) == 0 )
	{
		lua_State		*L = luaL_newstate();

		lua_moo::luaNewState( L );

		QByteArray		 P = mProgram.join( "\n" ).toUtf8();

		int Error = luaL_loadbuffer( L, P, P.size(), mVerbName.toUtf8() );

		if( Error == 0 )
		{
			mVerb->setScript( P );

			ObjectManager::instance()->markObject( mObject );
		}
		else
		{
			QString		Result = lua_tostring( L, -1 );

			mConnection->notify( Result );

			lua_pop( L, 1 );
		}

		lua_close( L );

		return( false );
	}

	mProgram.append( pData );

	return( true );
}
