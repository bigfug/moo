#include "property.h"
#include "objectmanager.h"

void Property::setPermissions( quint16 pPerms )
{
	mData.mRead   = ( pPerms & Property::READ );
	mData.mWrite  = ( pPerms & Property::WRITE );
	mData.mChange = ( pPerms & Property::CHANGE );

	setUpdated();
}

void Property::setParent(ObjectId pParent)
{
	if( mData.mParent != pParent )
	{
		mData.mParent = pParent;

		setUpdated();
	}
}

void Property::setOwner(ObjectId pOwner)
{
	if( mData.mOwner != pOwner )
	{
		mData.mOwner = pOwner;

		setUpdated();
	}
}

void Property::setValue(const QVariant &pValue)
{
	if( mData.mValue != pValue )
	{
		mData.mValue = pValue;

		setUpdated();
	}
}

void Property::setRead(bool pValue)
{
	if( mData.mRead != pValue )
	{
		mData.mRead = pValue;

		setUpdated();
	}
}

void Property::setWrite(bool pValue)
{
	if( mData.mWrite != pValue )
	{
		mData.mWrite = pValue;

		setUpdated();
	}
}

void Property::setChange(bool pValue)
{
	if( mData.mChange != pValue )
	{
		mData.mChange = pValue;

		setUpdated();
	}
}

quint16 Property::permissions( void ) const
{
	quint16			P = 0;

	if( mData.mRead   ) P |= Property::READ;
	if( mData.mWrite  ) P |= Property::WRITE;
	if( mData.mChange ) P |= Property::CHANGE;

	return( P );
}

void Property::setObject( ObjectId pObject )
{
	mData.mObject = pObject;
}

void Property::setName( QString pName )
{
	mData.mName = pName;
}

void Property::initialise( void )
{
	mData.mObject = OBJECT_NONE;
	mData.mParent = OBJECT_NONE;
	mData.mOwner  = OBJECT_NONE;
	mData.mRead   = false;
	mData.mWrite  = false;
	mData.mChange = true;

	mData.mValue.clear();
}

void Property::setUpdated()
{
	ObjectManager	*OM = ObjectManager::instance();
	Object			*O  = OM->o( mData.mObject );

	if( O )
	{
		OM->updateProperty( O, mData.mName );
	}
}
