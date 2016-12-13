#include "property.h"

void Property::setPermissions( quint16 pPerms )
{
	mData.mRead   = ( pPerms & Property::READ );
	mData.mWrite  = ( pPerms & Property::WRITE );
	mData.mChange = ( pPerms & Property::CHANGE );
}

quint16 Property::permissions( void ) const
{
	quint16			P = 0;

	if( mData.mRead   ) P |= Property::READ;
	if( mData.mWrite  ) P |= Property::WRITE;
	if( mData.mChange ) P |= Property::CHANGE;

	return( P );
}

void Property::initialise( void )
{
	mData.mParent = -1;
	mData.mOwner  = -1;
	mData.mRead   = false;
	mData.mWrite  = false;
	mData.mChange = true;

	mData.mValue.clear();
}
