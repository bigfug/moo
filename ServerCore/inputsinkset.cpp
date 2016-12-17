#include "inputsinkset.h"

#include "connection.h"
#include "object.h"
#include "property.h"
#include "lua_moo.h"
#include "objectmanager.h"

#include <lua.hpp>

InputSinkSet::InputSinkSet( Connection *C, Object *O, Property *P, const QString &pPropName )
{
	mConnection = C;
	mObject = O;
	mProperty = P;
	mPropName = pPropName;
}

bool InputSinkSet::input( const QString &pData )
{
	if( pData.compare( "." ) == 0 )
	{
		mProperty->setValue( mData.join( "\n" ).toUtf8() );

		return( false );
	}

	mData.append( pData );

	return( true );
}
