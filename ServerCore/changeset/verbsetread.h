#ifndef VERBSETREAD_H
#define VERBSETREAD_H

#include "../verb.h"

#include "change.h"

namespace change {

class VerbSetRead : public Change
{
public:
	VerbSetRead( Verb *pVerb, bool pValue )
		: mVerb( pVerb )
	{
		mOldValue = mVerb->read();

		mVerb->setRead( pValue );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mVerb->setRead( mOldValue );
	}

private:
	Verb		*mVerb;
	bool		 mOldValue;
};

}

#endif // VERBSETREAD_H
