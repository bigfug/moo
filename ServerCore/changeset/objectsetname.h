#ifndef OBJECTSETNAME_H
#define OBJECTSETNAME_H

#include "../object.h"

#include "change.h"

namespace change {

class ObjectSetName : public Change
{
public:
	ObjectSetName( Object *pObject, QString pName )
		: mObject( pObject )
	{
		mOldName = mObject->name();

		mObject->setName( pName );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mObject->setName( mOldName );
	}

private:
	Object		*mObject;
	QString		 mOldName;
};

}

#endif // OBJECTSETNAME_H
