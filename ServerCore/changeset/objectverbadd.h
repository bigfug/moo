#ifndef OBJECTVERBADD_H
#define OBJECTVERBADD_H

#include "../object.h"
#include "../verb.h"

#include "change.h"

namespace change {

class ObjectVerbAdd : public Change
{
public:
	ObjectVerbAdd( Object *pObject, QString pVerbName, Verb pVerbInit )
		: mObject( pObject ), mVerbName( pVerbName )
	{
		mObject->verbAdd( mVerbName, pVerbInit );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mObject->verbDelete( mVerbName );
	}

private:
	Object		*mObject;
	QString		 mVerbName;
};

}

#endif // OBJECTVERBADD_H
