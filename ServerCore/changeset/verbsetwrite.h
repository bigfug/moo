#ifndef VERBSETWRITE_H
#define VERBSETWRITE_H

#include "../verb.h"

#include "change.h"

namespace change {

class VerbSetWrite : public Change
{
public:
	VerbSetWrite( Verb *pVerb, bool pValue )
		: mVerb( pVerb )
	{
		mOldValue = mVerb->write();

		mVerb->setWrite( pValue );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mVerb->setWrite( mOldValue );
	}

private:
	Verb		*mVerb;
	bool		 mOldValue;
};

}

#endif // VERBSETWRITE_H
