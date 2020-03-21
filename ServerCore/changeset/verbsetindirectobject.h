#ifndef VERBSETINDIRECTOBJECT_H
#define VERBSETINDIRECTOBJECT_H

#include "../verb.h"

#include "change.h"

namespace change {

class VerbSetIndirectObject : public Change
{
public:
	VerbSetIndirectObject( Verb *pVerb, ArgObj pNewArg )
		: mVerb( pVerb )
	{
		mOldArg = mVerb->indirectObject();

		mVerb->setIndirectObjectArgument( pNewArg );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mVerb->setIndirectObjectArgument( mOldArg );
	}

private:
	Verb		*mVerb;
	ArgObj		 mOldArg;
};

}

#endif // VERBSETINDIRECTOBJECT_H
