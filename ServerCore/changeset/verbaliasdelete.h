#ifndef VERBALIASDELETE_H
#define VERBALIASDELETE_H

#include "../verb.h"

#include "change.h"

namespace change {

class VerbAliasDelete : public Change
{
public:
	VerbAliasDelete( Verb *pVerb, QString pAlias )
		: mVerb( pVerb ), mAlias( pAlias )
	{
		mVerb->remAlias( mAlias );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mVerb->addAlias( mAlias );
	}

private:
	Verb		*mVerb;
	QString		 mAlias;
};

}
#endif // VERBALIASDELETE_H
