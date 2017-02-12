#ifndef VERB_H
#define VERB_H

#include <QString>
#include <QDataStream>
#include <QStringList>

#include <lua.hpp>

#include "mooglobal.h"
#include "func.h"

typedef enum ArgObj
{
	THIS, ANY, NONE
} ArgObj;

typedef struct VerbData
{
	ArgObj			mDirectObject;
	ArgObj			mIndirectObject;
	ArgObj			mPrepositionType;
	QString			mPreposition;
	QStringList		mAliases;
} VerbData;

class Verb : public Func
{
	friend class ODB;

public:
	virtual ~Verb( void ) {}

	void initialise();

	static QStringList parse( const QString &pInput, QString &pArgStr );

	static bool matchPattern( const QString &Pattern, const QString &pMatch);

	static bool matchName( const QStringList &pPattern, const QString &pMatch );

	bool matchPreposition( const QString &pPreposition ) const;

	bool matchArgs( ObjectId pObjectId, ObjectId DirectObjectId, const QString &pPreposition, ObjectId IndirectObjectId ) const;

public:
	static const char *argobj_name( ArgObj pArgObj )
	{
		switch( pArgObj )
		{
			case THIS:
				return( "this" );

			case ANY:
				return( "any" );

			case NONE:
				return( "none" );
		}

		return( "unknown" );
	}

	static ArgObj argobj_from( const char *pArgNam, bool *pOK = 0 )
	{
		if( !strcmp( pArgNam, "this" ) )
		{
			if( pOK ) *pOK = true;

			return( THIS );
		}

		if( !strcmp( pArgNam, "any" ) )
		{
			if( pOK ) *pOK = true;

			return( ANY );
		}

		if( !strcmp( pArgNam, "none" ) )
		{
			if( pOK ) *pOK = true;

			return( NONE );
		}

		if( pOK ) *pOK = false;

		return( NONE );
	}

	inline ArgObj directObject( void ) const
	{
		return( mVerbData.mDirectObject );
	}

	inline ArgObj indirectObject( void ) const
	{
		return( mVerbData.mIndirectObject );
	}

	inline const QString &preposition( void ) const
	{
		return( mVerbData.mPreposition );
	}

	inline ArgObj prepositionType( void ) const
	{
		return( mVerbData.mPrepositionType );
	}

	void setDirectObjectArgument( ArgObj pArg );

	void setIndirectObjectArgument( ArgObj pArg );

	void setPrepositionArgument( ArgObj pArg );

	void setPrepositionArgument( const QString &pArg );

	const QStringList &aliases( void ) const
	{
		return( mVerbData.mAliases );
	}

	void addAlias( const QString &pAlias );
	void remAlias( const QString &pAlias );

protected:
	VerbData &data( void )
	{
		return( mVerbData );
	}

	const VerbData &data( void ) const
	{
		return( mVerbData );
	}

private:
	VerbData			mVerbData;
};


#endif // VERB_H
