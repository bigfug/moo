#ifndef OBJECTVERBDELETE_H
#define OBJECTVERBDELETE_H

#include "../object.h"
#include "../verb.h"

#include "change.h"

namespace change {

class ObjectVerbDelete : public Change
{
public:
	ObjectVerbDelete( Object *pObject, QString pVerbName )
		: mObject( pObject ), mVerbName( pVerbName )
	{
		mOldVerb = *mObject->verb( mVerbName );

		mObject->verbDelete( mVerbName );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mObject->verbAdd( mVerbName, mOldVerb );
	}

private:
	Object		*mObject;
	QString		 mVerbName;
	Verb		 mOldVerb;
};

}

#endif // OBJECTVERBDELETE_H
