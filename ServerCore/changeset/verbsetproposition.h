#ifndef VERBSETPROPOSITION_H
#define VERBSETPROPOSITION_H

#include "../verb.h"

#include "change.h"

namespace change {

class VerbSetPreposition : public Change
{
public:
	VerbSetPreposition( Verb *pVerb, ArgObj pNewArg )
		: mVerb( pVerb )
	{
		mOldArg = mVerb->prepositionType();

		mVerb->setPrepositionArgument( pNewArg );
	}

	VerbSetPreposition( Verb *pVerb, QString pProp )
		: mVerb( pVerb )
	{
		mOldPrp = mVerb->preposition();

		mVerb->setPrepositionArgument( pProp );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		if( !mOldPrp.isEmpty() )
		{
			mVerb->setPrepositionArgument( mOldPrp );
		}
		else
		{
			mVerb->setPrepositionArgument( mOldArg );
		}
	}

private:
	Verb		*mVerb;
	ArgObj		 mOldArg;
	QString		 mOldPrp;
};

}

#endif // VERBSETPROPOSITION_H
