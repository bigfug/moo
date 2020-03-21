#ifndef VERBALIASADD_H
#define VERBALIASADD_H

#include "../verb.h"

#include "change.h"

namespace change {

class VerbAliasAdd : public Change
{
public:
	VerbAliasAdd( Verb *pVerb, QString pAlias )
		: mVerb( pVerb ), mAlias( pAlias )
	{
		mVerb->addAlias( mAlias );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mVerb->remAlias( mAlias );
	}

private:
	Verb		*mVerb;
	QString		 mAlias;
};

}

#endif // VERBALIASADD_H
