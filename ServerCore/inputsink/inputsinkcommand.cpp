#include "inputsinkcommand.h"

InputSinkCommand::InputSinkCommand( Connection *C )
	: mConnection( C )
{

}

bool InputSinkCommand::input( const QString &pData )
{
	return( true );
}

bool InputSinkCommand::output( const QString &pData )
{
	Q_UNUSED( pData )

	return( false );
}
