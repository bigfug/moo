#include "func.h"
#include "lua_moo.h"

#include "objectmanager.h"

void Func::initialise( void )
{
	mData.mObject = OBJECT_NONE;
	mData.mOwner  = OBJECT_NONE;
	mData.mRead    = true;
	mData.mWrite  = false;
	mData.mExecute = true;
	mData.mScript.clear();
	mData.mCompiled.clear();
	mData.mDirty = false;
}

int Func::writerStatic( lua_State *L, const void *p, size_t sz, void *ud )
{
	return( reinterpret_cast<Func *>( ud )->writer( L, p, sz ) );
}

int Func::writer( lua_State *L, const void* p, size_t sz )
{
	Q_UNUSED( L )

	mData.mCompiled.append( reinterpret_cast<const char *>( p ), sz );

	return( 0 );
}

int Func::compile()
{
	qDebug() << "Compiling" << mData.mName;

	lua_State		*L = luaL_newstate();

	if( !L )
	{
		return( 0 );
	}

	lua_moo::luaNewState( L );

	int Error = luaL_loadstring( L, mData.mScript.toLatin1() );

	if( Error == 0 )
	{
		mData.mCompiled.clear();

		lua_dump( L, &Func::writerStatic, this );

		mData.mDirty = false;
	}

	//lua_moo::stackDump( L );

	lua_close( L );

	return( Error );
}

void Func::setPermissions( quint16 pPerms )
{
	mData.mRead    = ( pPerms & Func::READ );
	mData.mWrite   = ( pPerms & Func::WRITE );
	mData.mExecute = ( pPerms & Func::EXECUTE );

	setUpdated();
}

quint16 Func::permissions()
{
	quint16			P = 0;

	if( mData.mRead    ) P |= Func::READ;
	if( mData.mWrite   ) P |= Func::WRITE;
	if( mData.mExecute ) P |= Func::EXECUTE;

	return( P );
}

void Func::setObject( ObjectId pObject )
{
	mData.mObject = pObject;
}

void Func::setName( QString pName )
{
	mData.mName = pName;
}

void Func::setOwner(ObjectId pOwner)
{
	if( mData.mOwner != pOwner )
	{
		mData.mOwner = pOwner;

		setUpdated();
	}
}

void Func::setRead( bool pRead )
{
	if( mData.mRead != pRead )
	{
		mData.mRead = pRead;

		setUpdated();
	}
}

void Func::setWrite( bool pWrite )
{
	if( mData.mWrite != pWrite )
	{
		mData.mWrite = pWrite;

		setUpdated();
	}
}

void Func::setExecute( bool pExecute )
{
	if( mData.mExecute != pExecute )
	{
		mData.mExecute = pExecute;

		setUpdated();
	}
}

void Func::setScript( const QString &pScript )
{
	if( mData.mScript != pScript )
	{
		mData.mScript = pScript;
		mData.mDirty  = true;

//		if( compile() == 0 )
//		{
//			mData.mDirty = false;
//		}

		setUpdated();
	}
}

const char * Func::readerStatic( lua_State *L, void *data, size_t *size )
{
	return( reinterpret_cast<Func *>( data )->reader( L, size ) );
}

const char * Func::reader( lua_State *L, size_t *size )
{
	Q_UNUSED( L )

	*size = mData.mCompiled.size();

	return( mData.mCompiled );
}

void Func::setUpdated()
{
	ObjectManager	*OM = ObjectManager::instance();
	Object			*O  = OM->o( mData.mObject );

	if( O )
	{
		OM->updateVerb( O, mData.mName );
	}
}

int Func::lua_pushverb( lua_State *L )
{
	if( !mData.mDirty && mData.mCompiled.size() > 0 )
	{
		return( lua_load( L, &Func::readerStatic, this, "verb" ) );
	}

	return( luaL_loadstring( L, mData.mScript.toUtf8() ) );
}

