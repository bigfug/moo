#include "lua_verb.h"
#include "lua_moo.h"
#include "verb.h"
#include "lua_utilities.h"
#include "mooexception.h"
#include "lua_task.h"
#include "lua_object.h"
#include "object.h"
#include "objectmanager.h"
#include "connectionmanager.h"
#include "connection.h"
#include "inputsinkprogram.h"

const char	*lua_verb::mLuaName = "moo.verb";

LuaMap		lua_verb::mLuaMap;

const luaL_Reg lua_verb::mLuaStatic[] =
{
	{ 0, 0 }
};

const luaL_reg lua_verb::mLuaInstance[] =
{
	{ "__index", lua_verb::luaGet },
	{ "__newindex", lua_verb::luaSet },
	{ "__gc", lua_verb::luaGC },
	{ 0, 0 }
};

const luaL_Reg lua_verb::mLuaInstanceFunctions[] =
{
	{ "aliasadd", lua_verb::luaAliasAdd },
	{ "aliasrem", lua_verb::luaAliasRem },
	{ "dump", lua_verb::luaDump },
	{ "program", lua_verb::luaProgram },
	{ 0, 0 }
};

void lua_verb::initialise()
{
//	lua_moo::addFunctions( mLuaStatic );

	// As we're overriding __index, build a static QMap of commands
	// pointing to their relevant functions (hopefully pretty fast)

	for( const luaL_Reg *FP = mLuaInstanceFunctions ; FP->name != 0 ; FP++ )
	{
		mLuaMap[ FP->name ] = FP->func;
	}
}

void lua_verb::luaRegisterState( lua_State *L )
{
	// Create the moo.object metatables that is used for all objects

	luaL_newmetatable( L, mLuaName );

	lua_pushstring( L, "__index" );
	lua_pushvalue( L, -2 );  /* pushes the metatable */
	lua_settable( L, -3 );  /* metatable.__index = metatable */

	luaL_openlib( L, NULL, lua_verb::mLuaInstance, 0 );

	lua_pop( L, 1 );

	Q_ASSERT( lua_gettop( L ) == 0 );
}

void lua_verb::lua_pushverb( lua_State *L, Verb *V, const QString &pName, ObjectId pObjectId )
{
	luaVerb			*H = (luaVerb *)lua_newuserdata( L, sizeof( luaVerb ) );

	if( H == 0 )
	{
		luaL_error( L, "out of memory" );

		return;
	}

	H->mName     = new QString( pName );
	H->mObjectId = pObjectId;
	H->mVerb     = V;

	luaL_getmetatable( L, mLuaName );
	lua_setmetatable( L, -2 );
}

lua_verb::luaVerb *lua_verb::arg( lua_State *L, int pIndex )
{
	luaVerb *H = (luaVerb *)luaL_checkudata( L, pIndex, mLuaName );

	luaL_argcheck( L, H != NULL, pIndex, "`verb' expected" );

	return( H );
}

int lua_verb::luaGC( lua_State *L )
{
	luaVerb		*V = arg( L );

	if( V->mObjectId == OBJECT_NONE )
	{
		delete( V->mVerb );
	}

	V->mObjectId = OBJECT_NONE;
	V->mVerb     = 0;

	if( V->mName != 0 )
	{
		delete( V->mName );
	}

	return( 0 );
}

int lua_verb::luaGet( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		const Task			&T = lua_task::luaGetTask( L )->task();
		luaVerb				*LV = arg( L );
		Verb				*V = LV->mVerb;
		const char			*s = luaL_checkstring( L, 2 );
		Object				*O = ObjectManager::o( LV->mObjectId );
		Object				*Player = ObjectManager::instance()->object( T.player() );
		const bool			 isOwner  = ( Player != 0 && O != 0 ? Player->id() == O->owner() : false );
		const bool			 isWizard = ( Player != 0 ? Player->wizard() : false );

		if( V == 0 )
		{
			throw( mooException( E_TYPE, "invalid object" ) );
		}

		// Look for function in mLuaMap

		lua_CFunction	 F;

		if( ( F = mLuaMap.value( s, 0 ) ) != 0 )
		{
			lua_pushcfunction( L, F );

			return( 1 );
		}

		if( strcmp( s, "name" ) == 0 )
		{
			lua_pushstring( L, LV->mName->toLatin1() );

			return( 1 );
		}

		if( strcmp( s, "aliases" ) == 0 )
		{
			lua_pushstring( L, V->aliases().toLatin1() );

			return( 1 );
		}

		if( strcmp( s, "owner" ) == 0 )
		{
			lua_object::lua_pushobjectid( L, V->owner() );

			return( 1 );
		}

		if( strcmp( s, "r" ) == 0 )
		{
			lua_pushboolean( L, V->read() );

			return( 1 );
		}

		if( strcmp( s, "w" ) == 0 )
		{
			lua_pushboolean( L, V->write() );

			return( 1 );
		}

		if( strcmp( s, "x" ) == 0 )
		{
			lua_pushboolean( L, V->execute() );

			return( 1 );
		}

		if( strcmp( s, "script" ) == 0 )
		{
			if( !isWizard && !isOwner && !V->read() )
			{
				throw( mooException( E_TYPE, "not allowed to read script" ) );
			}

			lua_pushstring( L, V->script().toLatin1() );

			return( 1 );
		}

		// Nothing found

		throw( mooException( E_PROPNF, QString( "property '%1' is not defined" ).arg( QString( s ) ) ) );
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

int lua_verb::luaSet( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		lua_task			*Command = lua_task::luaGetTask( L );
		const Task			&T = Command->task();
		Object				*Player = ObjectManager::o( T.programmer() );

		if( Player == 0 )
		{
			throw mooException( E_PERM, "programmer is invalid" );
		}

		luaVerb				*LV = arg( L );

		if( LV == 0 )
		{
			throw( mooException( E_PERM, "verb is invalid" ) );
		}

		Verb				*V = LV->mVerb;
		Object				*O = ObjectManager::o( LV->mObjectId );
		const char			*N = luaL_checkstring( L, 2 );
		const bool			 isOwner  = ( Player != 0 && O != 0 ? Player->id() == O->owner() : false );
		const bool			 isWizard = ( Player != 0 ? Player->wizard() : false );

		if( strcmp( N, "name" ) == 0 )
		{
			throw( mooException( E_PERM, "can't set object id" ) );
		}

		if( strcmp( N, "owner" ) == 0 )
		{
			if( !isWizard && !isOwner )
			{
				throw( mooException( E_PERM, "player is not owner or wizard" ) );
			}

			Object				*D = lua_object::argObj( L, 3 );

			V->setOwner( D->id() );

			return( 0 );
		}

		if( strcmp( N, "r" ) == 0 )
		{
			if( !isWizard && !isOwner )
			{
				throw( mooException( E_PERM, "player is not owner or wizard" ) );
			}

			bool		v = lua_toboolean( L, 3 );

			V->setRead( v );

			return( 0 );
		}

		if( strcmp( N, "w" ) == 0 )
		{
			if( !isWizard && !isOwner )
			{
				throw( mooException( E_PERM, "player is not owner or wizard" ) );
			}

			bool		v = lua_toboolean( L, 3 );

			V->setWrite( v );

			return( 0 );
		}

		if( strcmp( N, "x" ) == 0 )
		{
			if( !isWizard && !isOwner )
			{
				throw( mooException( E_PERM, "player is not owner or wizard" ) );
			}

			bool		v = lua_toboolean( L, 3 );

			V->setExecute( v );

			return( 0 );
		}

		if( strcmp( N, "script" ) == 0 )
		{
			if( !isWizard && !isOwner )
			{
				throw( mooException( E_PERM, "player is not owner or wizard" ) );
			}

			V->setScript( lua_tostring( L, 3 ) );

			return( 0 );
		}

		if( strcmp( N, "dobj" ) == 0 )
		{
			if( !isWizard && !isOwner )
			{
				throw( mooException( E_PERM, "player is not owner or wizard" ) );
			}

			const QString	Direct = luaL_checkstring( L, 3 );

			if( Direct == "this" )
			{
				V->setDirectObjectArgument( Verb::THIS );
			}
			else if( Direct == "any" )
			{
				V->setDirectObjectArgument( Verb::ANY );
			}
			else if( Direct == "none" )
			{
				V->setDirectObjectArgument( Verb::NONE );
			}
			else
			{
				throw( mooException( E_PROPNF, QString( "unknown direct object argument" ).arg( Direct ) ) );
			}

			return( 0 );
		}

		if( strcmp( N, "iobj" ) == 0 )
		{
			if( !isWizard && !isOwner )
			{
				throw( mooException( E_PERM, "player is not owner or wizard" ) );
			}

			const QString	Direct = luaL_checkstring( L, 3 );

			if( Direct == "this" )
			{
				V->setIndirectObjectArgument( Verb::THIS );
			}
			else if( Direct == "any" )
			{
				V->setIndirectObjectArgument( Verb::ANY );
			}
			else if( Direct == "none" )
			{
				V->setIndirectObjectArgument( Verb::NONE );
			}
			else
			{
				throw( mooException( E_PROPNF, QString( "unknown indirect object argument" ).arg( Direct ) ) );
			}

			return( 0 );
		}

		if( strcmp( N, "prep" ) == 0 )
		{
			if( !isWizard && !isOwner )
			{
				throw( mooException( E_PERM, "player is not owner or wizard" ) );
			}

			const QString	Direct = luaL_checkstring( L, 3 );

			if( Direct == "this" )
			{
				V->setPrepositionArgument( Verb::THIS );
			}
			else if( Direct == "any" )
			{
				V->setPrepositionArgument( Verb::ANY );
			}
			else if( Direct == "none" )
			{
				V->setPrepositionArgument( Verb::NONE );
			}
			else
			{
				V->setPrepositionArgument( Direct );
			}

			return( 0 );
		}

		throw( mooException( E_PROPNF, QString( "property '%1' is not defined" ).arg( N ) ) );
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

int lua_verb::luaAliasAdd( lua_State *L )
{
	bool		LuaErr = false;

	luaL_checkstring( L, 2 );

	try
	{
		const Task			&T = lua_task::luaGetTask( L )->task();
		Object				*Player = ObjectManager::o( T.programmer() );

		if( Player == 0 )
		{
			throw mooException( E_PERM, "programmer is invalid" );
		}

		luaVerb				*LV = arg( L );

		if( LV == 0 )
		{
			throw( mooException( E_PERM, "verb is invalid" ) );
		}

		Verb				*V = LV->mVerb;
		const char			*N = luaL_checkstring( L, 2 );

		if( Player->id() != V->owner() && !Player->wizard() )
		{
			throw mooException( E_PERM, "programmer has no access" );
		}

		V->addAlias( QString( N ) );
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

int lua_verb::luaAliasRem(lua_State *L)
{
	bool		LuaErr = false;

	luaL_checkstring( L, 2 );

	try
	{
		const Task			&T = lua_task::luaGetTask( L )->task();
		Object				*Player = ObjectManager::o( T.programmer() );

		if( Player == 0 )
		{
			throw mooException( E_PERM, "programmer is invalid" );
		}

		luaVerb				*LV = arg( L );

		if( LV == 0 )
		{
			throw( mooException( E_PERM, "verb is invalid" ) );
		}

		Verb				*V = LV->mVerb;
		const char			*N = luaL_checkstring( L, 2 );

		if( Player->id() != V->owner() && !Player->wizard() )
		{
			throw mooException( E_PERM, "programmer has no access" );
		}

		V->remAlias( QString( N ) );
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

int lua_verb::luaDump( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		const Task			&T = lua_task::luaGetTask( L )->task();
		luaVerb				*LV = arg( L );
		Verb				*V = LV->mVerb;
		Object				*O = ObjectManager::o( LV->mObjectId );
		Object				*Player = ObjectManager::instance()->object( T.player() );
		const bool			 isOwner  = ( Player != 0 && O != 0 ? Player->id() == O->owner() : false );
		const bool			 isWizard = ( Player != 0 ? Player->wizard() : false );
		Connection			*C = ConnectionManager::instance()->connection( lua_task::luaGetTask( L )->connectionid() );

		if( V == 0 )
		{
			throw( mooException( E_TYPE, "invalid object" ) );
		}

		if( !isWizard && !isOwner && !V->read() )
		{
			throw( mooException( E_TYPE, "not allowed to read script" ) );
		}

		QStringList		Program = V->script().split( "\n" );

		for( QStringList::iterator it = Program.begin() ; it != Program.end() ; it++ )
		{
			C->notify( *it );
		}

		C->notify( "." );
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

int lua_verb::luaProgram( lua_State *L )
{
	bool		LuaErr = false;

	try
	{
		const Task			&T = lua_task::luaGetTask( L )->task();
		luaVerb				*LV = arg( L );
		Verb				*V = LV->mVerb;
		Object				*O = ObjectManager::o( LV->mObjectId );
		Object				*Player = ObjectManager::instance()->object( T.player() );
		const bool			 isOwner  = ( Player != 0 && O != 0 ? Player->id() == O->owner() : false );
		const bool			 isWizard = ( Player != 0 ? Player->wizard() : false );
		Connection			*C = ConnectionManager::instance()->connection( lua_task::luaGetTask( L )->connectionid() );

		if( V == 0 )
		{
			throw( mooException( E_TYPE, "invalid object" ) );
		}

		if( !isWizard && !isOwner && !V->read() )
		{
			throw( mooException( E_TYPE, "not allowed to read script" ) );
		}

		InputSinkProgram	*IS = new InputSinkProgram( C, O, V, *LV->mName );

		if( IS == 0 )
		{
			return( 0 );
		}

		C->pushInputSink( IS );
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
