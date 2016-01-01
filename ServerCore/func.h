#ifndef FUNC_H
#define FUNC_H

#include <QString>
#include <QDataStream>
#include <QStringList>

#include <lua.hpp>

#include "mooglobal.h"

class Func
{
public:
	virtual void save( QDataStream &pData ) const;
	virtual void load( QDataStream &pData );

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
		return( mObject );
	}

	inline ObjectId owner( void ) const
	{
		return( mOwner );
	}

	inline bool read( void ) const
	{
		return( mRead );
	}

	inline bool write( void ) const
	{
		return( mWrite );
	}

	inline bool execute( void ) const
	{
		return( mExecute );
	}

	inline const QString &script( void ) const
	{
		return( mScript );
	}

	inline const QByteArray &compiled( void ) const
	{
		return( mCompiled );
	}

	inline bool dirty( void ) const
	{
		return( mDirty );
	}

	inline void setObject( ObjectId pObject )
	{
		mObject = pObject;
	}

	inline void setOwner( ObjectId pOwner )
	{
		mOwner = pOwner;
	}

	inline void setRead( bool pRead )
	{
		mRead = pRead;
	}

	inline void setWrite( bool pWrite )
	{
		mWrite = pWrite;
	}

	inline void setExecute( bool pExecute )
	{
		mExecute = pExecute;
	}

	inline void setScript( const QString &pScript )
	{
		mScript = pScript;
		mDirty  = true;
	}

private:
	static int writerStatic( lua_State *L, const void* p, size_t sz, void* ud );
	int writer( lua_State *L, const void* p, size_t sz );

	static const char *readerStatic( lua_State *L, void *data, size_t *size );
	const char *reader( lua_State *L, size_t *size );

private:
	ObjectId		mObject;		// The object this func is attached to
	ObjectId		mOwner;
	bool			mRead;			// lets non-owners see the program for a verb
	bool			mWrite;			// lets them change that program
	bool			mExecute;		// whether or not the verb can be invoked from within a MOO program (as opposed to from the command line, like the `put' verb on containers)
	QString			mScript;
	QByteArray		mCompiled;
	bool			mDirty;
};

#endif // FUNC_H
