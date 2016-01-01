#include "property.h"

void Property::save( QDataStream &pData ) const
{
	pData << mOwner;
	pData << mRead;
	pData << mWrite;
	pData << mChange;
	pData << mValue;
}

void Property::load( QDataStream &pData )
{
	pData >> mOwner;
	pData >> mRead;
	pData >> mWrite;
	pData >> mChange;
	pData >> mValue;
}

void Property::setPermissions( quint16 pPerms )
{
	mRead   = ( pPerms & Property::READ );
	mWrite  = ( pPerms & Property::WRITE );
	mChange = ( pPerms & Property::CHANGE );
}

quint16 Property::permissions( void ) const
{
	quint16			P = 0;

	if( mRead   ) P |= Property::READ;
	if( mWrite  ) P |= Property::WRITE;
	if( mChange ) P |= Property::CHANGE;

	return( P );
}

void Property::initialise( void )
{
	mParent = -1;
	mOwner  = -1;
	mRead   = false;
	mWrite  = false;
	mChange = true;

	mValue.clear();
}
