#ifndef OBJECTSETPARENT_H
#define OBJECTSETPARENT_H

#include "../object.h"
#include "../objectmanager.h"

#include <QMultiMap>

#include "change.h"

namespace change {

class ObjectSetParent : public Change
{
public:
	ObjectSetParent( Object *pObject, ObjectId pNewParentId, QMultiMap<ObjectId,Property> pPrpMap )
		: mObject( pObject ), mNewParent( pNewParentId ), mPrpMap( pPrpMap )
	{
		mOldParent = pObject->parent();

		mObject->setParent( mNewParent );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
		// do nothing
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		mObject->setParent( mOldParent );

		for( ObjectId OID : mPrpMap.keys() )
		{
			Object		*O = ObjectManager::o( OID );

			if( !O )
			{
				continue;
			}

			for( Property P : mPrpMap.values( OID ) )
			{
				O->propAdd( P.name(), P );
			}
		}
	}

private:
	Object							*mObject;
	ObjectId						 mNewParent;
	ObjectId						 mOldParent;
	QMultiMap<ObjectId,Property>	 mPrpMap;
};

}

#endif // OBJECTSETPARENT_H
