#ifndef FUNC_H
#define FUNC_H

#include <QString>
#include <QStringList>

#include <lua.hpp>

#include "mooglobal.h"

typedef struct FuncData
{
	ObjectId		mObject;		// The object this func is attached to
	ObjectId		mOwner;
	bool			mRead;			// lets non-owners see the program for a verb
	bool			mWrite;			// lets them change that program
	bool			mExecute;		// whether or not the verb can be invoked from within a MOO program (as opposed to from the command line, like the `put' verb on containers)
	QString			mScript;
	QByteArray		mCompiled;
	bool			mDirty;
} FuncData;

class Func
{
	friend class ODB;

public:
	void initialise();

	int compile( void );

	typedef enum Permissions
	{
		READ    = ( 1 << 0 ),
		WRITE   = ( 1 << 1 ),
		EXECUTE = ( 1 << 2 )
	} Permissions;

	int lua_pushverb( lua_State *L );

	void setPermissions( quint16 pPerms );
	quint16 permissions( void );

	inline ObjectId object( void ) const
	{
		return( mData.mObject );
	}

	inline ObjectId owner( void ) const
	{
		return( mData.mOwner );
	}

	inline bool read( void ) const
	{
		return( mData.mRead );
	}

	inline bool write( void ) const
	{
		return( mData.mWrite );
	}

	inline bool execute( void ) const
	{
		return( mData.mExecute );
	}

	inline const QString &script( void ) const
	{
		return( mData.mScript );
	}

	inline const QByteArray &compiled( void ) const
	{
		return( mData.mCompiled );
	}

	inline bool dirty( void ) const
	{
		return( mData.mDirty );
	}

	inline void setObject( ObjectId pObject )
	{
		mData.mObject = pObject;
	}

	inline void setOwner( ObjectId pOwner )
	{
		mData.mOwner = pOwner;
	}

	inline void setRead( bool pRead )
	{
		mData.mRead = pRead;
	}

	inline void setWrite( bool pWrite )
	{
		mData.mWrite = pWrite;
	}

	inline void setExecute( bool pExecute )
	{
		mData.mExecute = pExecute;
	}

	inline void setScript( const QString &pScript )
	{
		mData.mScript = pScript;
		mData.mDirty  = true;
	}

private:
	static int writerStatic( lua_State *L, const void* p, size_t sz, void* ud );
	int writer( lua_State *L, const void* p, size_t sz );

	static const char *readerStatic( lua_State *L, void *data, size_t *size );
	const char *reader( lua_State *L, size_t *size );

protected:
	FuncData &data( void )
	{
		return( mData );
	}

	const FuncData &data( void ) const
	{
		return( mData );
	}

private:
	FuncData			 mData;
};

#endif // FUNC_H
