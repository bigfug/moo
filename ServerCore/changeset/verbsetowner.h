#ifndef VERBSETOWNER_H
#define VERBSETOWNER_H

#include "../verb.h"

#include "change.h"

namespace change {

class VerbSetOwner : public Change
{
public:
	VerbSetOwner( Verb *pVerb, ObjectId pOwnerId )
		: mVerb( pVerb )
	{
		mOldOwner = mVerb->owner();

		mVerb->setOwner( pOwnerId );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mVerb->setOwner( mOldOwner );
	}

private:
	Verb		*mVerb;
	ObjectId	 mOldOwner;
};

}

#endif // VERBSETOWNER_H
