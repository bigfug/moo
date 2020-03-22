#ifndef OBJECTSETWIZARD_H
#define OBJECTSETWIZARD_H

#include "../object.h"

#include "change.h"

namespace change {

class ObjectSetWizard : public Change
{
public:
	ObjectSetWizard( Object *pObject, bool pValue )
		: mObject( pObject )
	{
		mOldValue = mObject->wizard();

		mObject->setWizard( pValue );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mObject->setWizard( mOldValue );
	}

private:
	Object		*mObject;
	bool		 mOldValue;
};

}

#endif // OBJECTSETWIZARD_H
