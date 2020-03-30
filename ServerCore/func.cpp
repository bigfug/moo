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
//	if( !mData.mName.isEmpty() )
//	{
//		qDebug() << "Compiling" << QString( "%1:%2" ).arg( mData.mObject ).arg( mData.mName );
//	}

	mData.mCompiled.clear();

//	lua_State		*L = luaL_newstate();

//	if( !L )
//	{
//		return( 0 );
//	}

//	qint64		CompileStart = QDateTime::currentMSecsSinceEpoch();

//	lua_moo::luaNewState( L );

//	int Error = luaL_loadstring( L, mData.mScript.toLatin1() );

//	if( !Error )
//	{
//		lua_dump( L, &Func::writerStatic, this );

//		mData.mDirty = false;
//	}

//	qint64		CompileEnd = QDateTime::currentMSecsSinceEpoch();

//	ObjectManager::instance()->recordCompilationTime( CompileEnd - CompileStart );

//	//lua_moo::stackDump( L );

//	lua_close( L );

//	return( Error );

	return( 0 );
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
		mData.mCompiled.clear();
		mData.mDirty  = true;

		if( !mData.mScript.isEmpty() )
		{
			compile();
		}

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
//	if( !mData.mDirty && mData.mCompiled.size() > 0 )
//	{
//		return( lua_load( L, &Func::readerStatic, this, "verb" ) );
//	}

	if( luaL_loadstring( L, mData.mScript.toUtf8() ) != 0 )
	{
		return( -1 );
	}

	lua_getglobal( L, "moo_sandbox" );

	lua_setupvalue( L, -2, 1 );

	return( 0 );
}

