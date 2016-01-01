#ifndef VERB_H
#define VERB_H

#include <QString>
#include <QDataStream>
#include <QStringList>

#include <lua.hpp>

#include "mooglobal.h"
#include "func.h"

class Verb : public Func
{
public:
	virtual ~Verb( void ) {}

	void save( QDataStream &pData ) const;
	void load( QDataStream &pData );

	void initialise();

	typedef enum ArgObj
	{
		THIS, ANY, NONE
	} ArgObj;

	static QStringList parse( const QString &pInput, QString &pArgStr );

	static bool matchName( const QString &pPattern, const QString &pMatch );

	bool matchPreposition( const QString &pPreposition );

	bool matchArgs( ObjectId pObjectId, ObjectId DirectObjectId, const QString &pPreposition, ObjectId IndirectObjectId );

public:
	inline ArgObj directObject( void ) const
	{
		return( mDirectObject );
	}

	inline ArgObj indirectObject( void ) const
	{
		return( mIndirectObject );
	}

	inline const QString &preposition( void ) const
	{
		return( mPreposition );
	}

	inline ArgObj prepositionType( void ) const
	{
		return( mPrepositionType );
	}

	inline void setDirectObjectArgument( ArgObj pArg )
	{
		mDirectObject = pArg;
	}

	inline void setIndirectObjectArgument( ArgObj pArg )
	{
		mIndirectObject = pArg;
	}

	inline void setPrepositionArgument( ArgObj pArg )
	{
		Q_ASSERT( pArg == Verb::ANY || pArg == Verb::NONE );

		mPrepositionType = pArg;
		mPreposition.clear();
	}

	inline void setPrepositionArgument( const QString &pArg )
	{
		mPrepositionType = Verb::THIS;
		mPreposition = pArg;
	}

	const QString &aliases( void ) const
	{
		return( mAliases );
	}

	void addAlias( const QString &pAlias );
	void remAlias( const QString &pAlias );

private:
	ArgObj			mDirectObject;
	ArgObj			mIndirectObject;
	ArgObj			mPrepositionType;
	QString			mPreposition;
	QString			mAliases;
};


#endif // VERB_H
