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
#include "inputsinkeditor.h"

#include "changeset/verbsetowner.h"
#include "changeset/verbsetread.h"
#include "changeset/verbsetwrite.h"
#include "changeset/verbsetexecute.h"
#include "changeset/verbsetscript.h"
#include "changeset/verbsetdirectobject.h"
#include "changeset/verbsetindirectobject.h"
#include "changeset/verbsetproposition.h"
#include "changeset/verbaliasadd.h"
#include "changeset/verbaliasdelete.h"

#include "changeset/connectionnotify.h"

const char	*lua_verb::mLuaName = "moo.verb";

LuaMap		lua_verb::mLuaMap;

const luaL_Reg lua_verb::mLuaStatic[] =
{
	{ 0, 0 }
};

const luaL_Reg lua_verb::mLuaInstance[] =
{
	{ "__index", lua_verb::luaGet },
	{ "__newindex", lua_verb::luaSet },
	{ "__gc", lua_verb::luaGC },
	{ "__call", lua_verb::luaCall },
	{ 0, 0 }
};

const luaL_Reg lua_verb::mLuaInstanceFunctions[] =
{
	{ "aliasadd", lua_verb::luaAliasAdd },
	{ "aliasrem", lua_verb::luaAliasRem },
	{ "aliasdel", lua_verb::luaAliasRem },
	{ "dump", lua_verb::luaDump },
	{ "program", lua_verb::luaProgram },
	{ "edit", lua_verb::luaEdit },
	{ 0, 0 }
};

const QMap<QString,lua_verb::Fields>		lua_verb::mFieldMap =
{
	{ "name", NAME },
	{ "aliases", ALIASES },
	{ "owner", OWNER },
	{ "r", READ },
	{ "w", WRITE },
	{ "x", EXECUTE },
	{ "read", READ },
	{ "write", WRITE },
	{ "execute", EXECUTE },
	{ "script", SCRIPT },
	{ "dobj", DIRECT_OBJECT },
	{ "iobj", INDIRECT_OBJECT },
	{ "prep", PREPOSITION }
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

	// metatable.__index = metatable
	lua_pushvalue( L, -1 ); // duplicates the metatable
	lua_setfield( L, -2, "__index" );

	luaL_setfuncs( L, mLuaInstance, 0 );

	lua_pop( L, 1 );

	Q_ASSERT( lua_gettop( L ) == 0 );
}

void lua_verb::lua_pushverb( lua_State *L, Verb *V )
{
	luaVerb			*H = (luaVerb *)lua_newuserdata( L, sizeof( luaVerb ) );

	if( H == 0 )
	{
		luaL_error( L, "out of memory" );

		return;
	}

//	H->mName     = new QString( pName );
//	H->mObjectId = pObjectId;
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

	if( V->mVerb->object() == OBJECT_NONE )
	{
		delete( V->mVerb );
	}

//	V->mObjectId = OBJECT_NONE;
	V->mVerb     = 0;

//	if( V->mName != 0 )
//	{
//		delete( V->mName );
//	}

	return( 0 );
}

int lua_verb::luaCall( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	try
	{
		luaVerb				*LV = arg( L );
		Verb				*V = LV->mVerb;
		Task				 T = Command->task();

		if( !V )
		{
			throw( mooException( E_VERBNF, "invalid verb" ) );
		}

		T.setCaller( V->object() );
		T.setObject( V->object() );
		T.setVerb( V->name() );

		if( !Command->isWizard() )
		{
			T.setPermissions( V->owner() );
		}

		return( Command->verbCall( T, V, lua_gettop( L ) - 1 ) );
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{
	}

	return( Command->lua_pushexception() );
}

int lua_verb::luaGet( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );


	try
	{
		const Task			&T = Command->task();
		luaVerb				*LV = arg( L );
		Verb				*V = LV->mVerb;
		const char			*s = luaL_checkstring( L, 2 );
		Object				*O = ObjectManager::o( V->object() );
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

		switch( mFieldMap.value( QString( s ) ) )
		{
			case NAME:
				{
					lua_pushstring( L, V->name().toLatin1() );

					return( 1 );
				}
				break;

			case ALIASES:
				{
					const QStringList	AliasList = V->aliases();

					lua_newtable( L );

					for( int i = 0 ; i < AliasList.size() ; i++ )
					{
						lua_pushstring( L, AliasList[ i ].toLatin1() );

						lua_rawseti( L, -2, i + 1 );
					}

					return( 1 );
				}
				break;

			case OWNER:
				{
					lua_object::lua_pushobjectid( L, V->owner() );

					return( 1 );
				}
				break;

			case READ:
				{
					lua_pushboolean( L, V->read() );

					return( 1 );
				}
				break;

			case WRITE:
				{
					lua_pushboolean( L, V->write() );

					return( 1 );
				}
				break;

			case EXECUTE:
				{
					lua_pushboolean( L, V->execute() );

					return( 1 );
				}
				break;

			case SCRIPT:
				{
					if( !isWizard && !isOwner && !V->read() )
					{
						throw( mooException( E_TYPE, "not allowed to read script" ) );
					}

					lua_pushstring( L, V->script().toLatin1() );

					return( 1 );
				}
				break;

			case DIRECT_OBJECT:
				{
					lua_pushstring( L, Verb::argobj_name( V->directObject() ) );

					return( 1 );
				}
				break;

			case INDIRECT_OBJECT:
				{
					lua_pushstring( L, Verb::argobj_name( V->indirectObject() ) );

					return( 1 );
				}
				break;

			case PREPOSITION:
				{
					QString		Prep = V->preposition();

					if( Prep.isEmpty() )
					{
						lua_pushstring( L, Verb::argobj_name( V->prepositionType() ) );
					}
					else
					{
						lua_pushfstring( L, "%s", Prep.toLatin1().constData() );
					}

					return( 1 );
				}
				break;

			case UNKNOWN:
				break;
		}

		// Nothing found

		throw( mooException( E_PROPNF, QString( "property '%1' is not defined" ).arg( QString( s ) ) ) );
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{
	}

	return( Command->lua_pushexception() );
}

int lua_verb::luaSet( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	try
	{
		if( !Command->isPermValid() )
		{
			throw mooException( E_PERM, "programmer is invalid" );
		}

		luaVerb				*LV = arg( L );

		if( !LV )
		{
			throw( mooException( E_PERM, "verb is invalid" ) );
		}

		Verb				*V = LV->mVerb;
		Object				*O = ObjectManager::o( V->object() );
		const char			*N = luaL_checkstring( L, 2 );

		switch( mFieldMap.value( QString( N ) ) )
		{
			case NAME:
				{
					throw( mooException( E_PERM, "can't set verb name" ) );
				}
				break;

			case ALIASES:
				{
					throw( mooException( E_PERM, "can't set verb alises here" ) );
				}
				break;

			case OWNER:
				{
					if( !Command->isWizard() && !Command->isOwner( O ) )
					{
						throw( mooException( E_PERM, "player is not owner or wizard" ) );
					}

					Object				*D = lua_object::argObj( L, 3 );

					Command->changeAdd( new change::VerbSetOwner( V, D->id() ) );

					return( 0 );
				}
				break;

			case READ:
				{
					if( !Command->isWizard() && !Command->isOwner( O ) )
					{
						throw( mooException( E_PERM, "player is not owner or wizard" ) );
					}

					bool		v = lua_toboolean( L, 3 );

					Command->changeAdd( new change::VerbSetRead( V, v ) );

					return( 0 );
				}
				break;

			case WRITE:
				{
					if( !Command->isWizard() && !Command->isOwner( O ) )
					{
						throw( mooException( E_PERM, "player is not owner or wizard" ) );
					}

					bool		v = lua_toboolean( L, 3 );

					Command->changeAdd( new change::VerbSetWrite( V, v ) );

					return( 0 );
				}
				break;

			case EXECUTE:
				{
					if( !Command->isWizard() && !Command->isOwner( O ) )
					{
						throw( mooException( E_PERM, "player is not owner or wizard" ) );
					}

					bool		v = lua_toboolean( L, 3 );

					Command->changeAdd( new change::VerbSetExecute( V, v ) );

					return( 0 );
				}
				break;

			case SCRIPT:
				{
					if( !Command->isWizard() && !Command->isOwner( O ) )
					{
						throw( mooException( E_PERM, "player is not owner or wizard" ) );
					}

					QString		v = lua_tostring( L, 3 );

					Command->changeAdd( new change::VerbSetScript( V, v ) );

					return( 0 );
				}
				break;

			case DIRECT_OBJECT:
				{
					if( !Command->isWizard() && !Command->isOwner( O ) )
					{
						throw( mooException( E_PERM, "player is not owner or wizard" ) );
					}

					const QString	Direct = luaL_checkstring( L, 3 );

					if( Direct == "this" )
					{
						Command->changeAdd( new change::VerbSetDirectObject( V, THIS ) );

						//V->setDirectObjectArgument( THIS );
					}
					else if( Direct == "any" )
					{
						Command->changeAdd( new change::VerbSetDirectObject( V, ANY ) );

		//				V->setDirectObjectArgument( ANY );
					}
					else if( Direct == "none" )
					{
						Command->changeAdd( new change::VerbSetDirectObject( V, NONE ) );

		//				V->setDirectObjectArgument( NONE );
					}
					else
					{
						throw( mooException( E_PROPNF, QString( "unknown direct object argument" ).arg( Direct ) ) );
					}

					return( 0 );
				}
				break;

			case INDIRECT_OBJECT:
				{
					if( !Command->isWizard() && !Command->isOwner( O ) )
					{
						throw( mooException( E_PERM, "player is not owner or wizard" ) );
					}

					const QString	Direct = luaL_checkstring( L, 3 );

					if( Direct == "this" )
					{
						Command->changeAdd( new change::VerbSetIndirectObject( V, THIS ) );
					}
					else if( Direct == "any" )
					{
						Command->changeAdd( new change::VerbSetIndirectObject( V, ANY ) );
					}
					else if( Direct == "none" )
					{
						Command->changeAdd( new change::VerbSetIndirectObject( V, NONE ) );
					}
					else
					{
						throw( mooException( E_PROPNF, QString( "unknown indirect object argument" ).arg( Direct ) ) );
					}

					return( 0 );
				}
				break;

			case PREPOSITION:
				{
					if( !Command->isWizard() && !Command->isOwner( O ) )
					{
						throw( mooException( E_PERM, "player is not owner or wizard" ) );
					}

					const char		*Prep = luaL_checkstring( L, 3 );
					bool			 PrepTypeOK;
					ArgObj			 PrepType = Verb::argobj_from( Prep, &PrepTypeOK );

					if( PrepTypeOK )
					{
						if( PrepType == ANY )
						{
							Command->changeAdd( new change::VerbSetPreposition( V, ANY ) );
						}
						else if( PrepType == NONE )
						{
							Command->changeAdd( new change::VerbSetPreposition( V, NONE ) );
						}
						else
						{
							throw( mooException( E_INVARG, QString( "bad prep type: %1" ).arg( Prep ) ) );
						}
					}
					else
					{
						Command->changeAdd( new change::VerbSetPreposition( V, QString( Prep ) ) );
					}

					return( 0 );
				}
				break;

			case UNKNOWN:
				break;
		}

		throw( mooException( E_PROPNF, QString( "property '%1' is not defined" ).arg( N ) ) );
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{
	}

	return( Command->lua_pushexception() );
}

int lua_verb::luaAliasAdd( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	luaL_checkstring( L, 2 );

	try
	{
		if( !Command->isPermValid() )
		{
			throw mooException( E_PERM, "programmer is invalid" );
		}

		luaVerb				*LV = arg( L );

		if( !LV )
		{
			throw( mooException( E_PERM, "verb is invalid" ) );
		}

		Verb				*V = LV->mVerb;
		const char			*N = luaL_checkstring( L, 2 );

		if( !Command->isOwner( V ) && !Command->isWizard() )
		{
			throw mooException( E_PERM, "programmer has no access" );
		}

		Command->changeAdd( new change::VerbAliasAdd( V, N ) );
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{
	}

	return( Command->lua_pushexception() );
}

int lua_verb::luaAliasRem(lua_State *L)
{
	lua_task				*Command = lua_task::luaGetTask( L );

	luaL_checkstring( L, 2 );

	try
	{
		if( !Command->isPermValid() )
		{
			throw mooException( E_PERM, "programmer is invalid" );
		}

		luaVerb				*LV = arg( L );

		if( !LV )
		{
			throw( mooException( E_PERM, "verb is invalid" ) );
		}

		Verb				*V = LV->mVerb;
		const char			*N = luaL_checkstring( L, 2 );

		if( !Command->isOwner( V ) && !Command->isWizard() )
		{
			throw mooException( E_PERM, "programmer has no access" );
		}

		Command->changeAdd( new change::VerbAliasDelete( V, N ) );
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{
	}

	return( Command->lua_pushexception() );
}

int lua_verb::luaDump( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );

	try
	{
		const Task			&T = Command->task();
		luaVerb				*LV = arg( L );
		Verb				*V = LV->mVerb;
		Object				*O = ObjectManager::o( V->object() );
		Object				*Player = ObjectManager::instance()->object( T.player() );
		const bool			 isOwner  = ( Player != 0 && O != 0 ? Player->id() == O->owner() : false );
		const bool			 isWizard = ( Player != 0 ? Player->wizard() : false );
		Connection			*C = ConnectionManager::instance()->connection( Command->connectionId() );

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
			Command->changeAdd( new change::ConnectionNotify( C, *it ) );
		}

		Command->changeAdd( new change::ConnectionNotify( C, "." ) );
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{
	}

	return( Command->lua_pushexception() );
}

int lua_verb::luaProgram( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );


	try
	{
		const Task			&T = Command->task();
		luaVerb				*LV = arg( L );
		Verb				*V = LV->mVerb;
		Object				*O = ObjectManager::o( V->object() );
		Object				*Player = ObjectManager::instance()->object( T.player() );
		const bool			 isOwner  = ( Player != 0 && O != 0 ? Player->id() == O->owner() : false );
		const bool			 isWizard = ( Player != 0 ? Player->wizard() : false );
		Connection			*C = ConnectionManager::instance()->connection( Command->connectionId() );

		if( V == 0 )
		{
			throw( mooException( E_TYPE, "invalid object" ) );
		}

		if( !isWizard && !isOwner && !V->read() )
		{
			throw( mooException( E_TYPE, "not allowed to read script" ) );
		}

		InputSinkProgram	*IS = new InputSinkProgram( C, O->id(), V->name() );

		if( IS == 0 )
		{
			return( 0 );
		}

		C->pushInputSink( IS );
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{
	}

	return( Command->lua_pushexception() );
}

int lua_verb::luaEdit( lua_State *L )
{
	lua_task				*Command = lua_task::luaGetTask( L );


	try
	{
		const Task			&T = Command->task();
		luaVerb				*LV = arg( L );
		Verb				*V = LV->mVerb;
		Object				*O = ObjectManager::o( V->object() );
		Object				*Player = ObjectManager::instance()->object( T.player() );
		const bool			 isOwner  = ( Player != 0 && O != 0 ? Player->id() == O->owner() : false );
		const bool			 isWizard = ( Player != 0 ? Player->wizard() : false );
		Connection			*C = ConnectionManager::instance()->connection( Command->connectionId() );

		if( !V )
		{
			throw( mooException( E_TYPE, "invalid object" ) );
		}

		if( !isWizard && !isOwner && !V->read() )
		{
			throw( mooException( E_NACC, "not allowed to read script" ) );
		}

		if( !C->supportsLineMode() )
		{
			throw( mooException( E_INVARG, "terminal doesn't support linemode" ) );
		}

		QStringList		Program = V->script().split( "\n" );

		InputSinkEditor	*IS = new InputSinkEditor( C, O->id(), V->name(), Program );

		if( !IS )
		{
			return( 0 );
		}

		C->pushInputSink( IS );
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{
	}

	return( Command->lua_pushexception() );
}
