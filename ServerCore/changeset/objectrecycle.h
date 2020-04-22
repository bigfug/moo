#ifndef OBJECTRECYCLE_H
#define OBJECTRECYCLE_H

#include "../object.h"
#include "../objectmanager.h"

#include "change.h"

namespace change {

class ObjectRecycle : public Change
{
public:
	ObjectRecycle( ObjectId pObjectId )
		: mObjectId( pObjectId )
	{
		ObjectManager::instance()->recycle( mObjectId );
	}

	// Change interface
public:
	virtual void commit() Q_DECL_OVERRIDE
	{
	}

	virtual void rollback() Q_DECL_OVERRIDE
	{
		ObjectManager::instance()->restore( mObjectId );

		Object		*objObject = ObjectManager::o( mObjectId );
		Object		*objOwner  = objObject ? ObjectManager::o( objObject->owner() ) : nullptr;

		// if the owner of the former object has a property named `ownership_quota' and the value of that property is a integer, then recycle() treats that value as a quota and increments it by one, storing the result back into the `ownership_quota' property.

		if( objOwner )
		{
			Property		*Quota = objOwner->prop( "ownership_quota" );

			if( Quota && Quota->type() == QVariant::Double )
			{
				int		QuotaValue = Quota->value().toInt();

				if( QuotaValue > 0 )
				{
					objOwner->propSet( Quota->name(), double( QuotaValue - 1 ) );
				}
			}
		}
	}

private:
	ObjectId		 mObjectId;
};

}

#endif // OBJECTRECYCLE_H
