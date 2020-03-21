#ifndef VERBSETEXECUTE_H
#define VERBSETEXECUTE_H
#include "../verb.h"

#include "change.h"

namespace change {

class VerbSetExecute : public Change
{
public:
	VerbSetExecute( Verb *pVerb, bool pValue )
		: mVerb( pVerb )
	{
		mOldValue = mVerb->execute();

		mVerb->setExecute( pValue );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mVerb->setExecute( mOldValue );
	}

private:
	Verb		*mVerb;
	bool		 mOldValue;
};

}

#endif // VERBSETEXECUTE_H
