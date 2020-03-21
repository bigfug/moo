#ifndef VERBSETDIRECTOBJECT_H
#define VERBSETDIRECTOBJECT_H

#include "../verb.h"

#include "change.h"

namespace change {

class VerbSetDirectObject : public Change
{
public:
	VerbSetDirectObject( Verb *pVerb, ArgObj pNewArg )
		: mVerb( pVerb )
	{
		mOldArg = mVerb->directObject();

		mVerb->setDirectObjectArgument( pNewArg );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mVerb->setDirectObjectArgument( mOldArg );
	}

private:
	Verb		*mVerb;
	ArgObj		 mOldArg;
};

}

#endif // VERBSETDIRECTOBJECT_H
