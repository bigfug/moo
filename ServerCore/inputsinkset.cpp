#include "inputsinkset.h"

#include "connection.h"
#include "object.h"
#include "property.h"
#include "lua_moo.h"
#include "objectmanager.h"

#include <lua.hpp>

InputSinkSet::InputSinkSet( Connection *C, ObjectId pObjectId, QString pPropName )
	: mConnection( C ), mObjectId( pObjectId ), mPropName( pPropName )
{
}

bool InputSinkSet::input( const QString &pData )
{
	if( pData.compare( "." ) == 0 )
	{
		Object		*O = ObjectManager::o( mObjectId );
		Property	*P = ( O ? O->prop( mPropName ) : nullptr );

		if( P )
		{
			P->setValue( mData.join( "\n" ).toUtf8() );
		}

		return( false );
	}

	mData.append( pData );

	return( true );
}
