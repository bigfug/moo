#ifndef VERBSETSCRIPT_H
#define VERBSETSCRIPT_H

#include "../verb.h"

#include "change.h"

namespace change {

class VerbSetScript : public Change
{
public:
	VerbSetScript( Verb *pVerb, QString pValue )
		: mVerb( pVerb )
	{
		mOldValue = mVerb->script();

		mVerb->setScript( pValue );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mVerb->setScript( mOldValue );
	}

private:
	Verb		*mVerb;
	QString		 mOldValue;
};

}

#endif // VERBSETSCRIPT_H
