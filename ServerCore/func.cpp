#include "func.h"
#include "lua_moo.h"

void Func::save( QDataStream &pData ) const
{
	pData << mOwner;
	pData << mRead;
	pData << mWrite;
	pData << mExecute;
	pData << mScript;
}

void Func::load( QDataStream &pData )
{
	pData >> mOwner;
	pData >> mRead;
	pData >> mWrite;
	pData >> mExecute;
	pData >> mScript;

	mDirty = true;
}

void Func::initialise( void )
{
	mObject = OBJECT_NONE;
	mOwner  = OBJECT_NONE;
	mRead    = true;
	mWrite  = false;
	mExecute = true;
	mScript.clear();
	mCompiled.clear();
	mDirty = false;
}

int Func::writerStatic(lua_State *L, const void *p, size_t sz, void *ud)
{
	return( reinterpret_cast<Func *>( ud )->writer( L, p, sz ) );
}

int Func::writer( lua_State *L, const void* p, size_t sz )
{
	Q_UNUSED( L )

	mCompiled.append( reinterpret_cast<const char *>( p ), sz );

	return( 0 );
}

int Func::compile()
{
	lua_State		*L = luaL_newstate();

	if( L == 0 )
	{
		return( 0 );
	}

	lua_moo::luaNewState( L );

	int Error = luaL_loadstring( L, mScript.toLatin1() );

	if( Error == 0 )
	{
		mCompiled.clear();

		lua_dump( L, &Func::writerStatic, this );

		mDirty = false;
	}

	//lua_moo::stackDump( L );

	lua_close( L );

	return( Error );
}

void Func::setPermissions( quint16 pPerms )
{
	mRead    = ( pPerms & Func::READ );
	mWrite   = ( pPerms & Func::WRITE );
	mExecute = ( pPerms & Func::EXECUTE );
}

quint16 Func::permissions()
{
	quint16			P = 0;

	if( mRead    ) P |= Func::READ;
	if( mWrite   ) P |= Func::WRITE;
	if( mExecute ) P |= Func::EXECUTE;

	return( P );
}

const char * Func::readerStatic( lua_State *L, void *data, size_t *size )
{
	return( reinterpret_cast<Func *>( data )->reader( L, size ) );
}

const char * Func::reader( lua_State *L, size_t *size )
{
	Q_UNUSED( L )

	*size = mCompiled.size();

	return( mCompiled );
}

int Func::lua_pushverb( lua_State *L )
{
	if( !mDirty && mCompiled.size() > 0 )
	{
		return( lua_load( L, &Func::readerStatic, this, "verb" ) );
	}

	return( luaL_loadstring( L, mScript.toUtf8() ) );
}

