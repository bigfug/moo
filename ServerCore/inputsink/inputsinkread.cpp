#include "inputsinkread.h"

#include "connection.h"
#include "object.h"
#include "verb.h"
#include "mooapp.h"
#include "lua_moo.h"
#include "lua_object.h"
#include "lua_task.h"
#include "objectmanager.h"
#include "connection.h"

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

InputSinkRead::InputSinkRead( Connection *C, const Task &pTask, ObjectId pObjectId, QString pVerbName, QVariantMap pReadArgs, QVariantList pVerbArgs )
	: mConnection( C ), mTask( pTask ), mObjectId( pObjectId ), mVerbName( pVerbName ), mReadArgs( pReadArgs ), mVerbArgs( pVerbArgs ),
	  mReadDone( false )
{
	initialise();
}

InputSinkRead::InputSinkRead( Connection *C, const Task &pTask, QVariantMap pReadArgs, QVariantList pVerbArgs )
	: mConnection( C ), mTask( pTask ), mObjectId( pTask.object() ), mVerbName( pTask.verb() ), mReadArgs( pReadArgs ), mVerbArgs( pVerbArgs ),
	  mReadDone( false )
{
	initialise();
}

void InputSinkRead::initialise()
{
	if( mReadArgs.value( "password", false ).toBool() )
	{
		mLineEdit.setSecretChar( '#' );
	}

	connect( &mLineEdit, &LineEdit::lineOutput, [=]( const QByteArray &pLine )
	{
		mConnection->write( "\r\n" );

		Object		*O = ObjectManager::o( mObjectId );
		Verb		*V = ( O ? O->verb( mVerbName ) : nullptr );

		if( V )
		{
			lua_task	 L( mConnection->id(), mTask );

			L.setPermissions( V->owner() );

			int			ArgCnt = 1;

			lua_pushlstring( L.L(), pLine.constData(), pLine.size() );

			for( const QVariant &V : mVerbArgs )
			{
				switch( QMetaType::Type( V.type() ) )
				{
					case QMetaType::Bool:
						lua_pushboolean( L.L(), V.toBool() );
						ArgCnt++;
						break;

					case QMetaType::Double:
						lua_pushnumber( L.L(), V.toDouble() );
						ArgCnt++;
						break;

					case QMetaType::Float:
						lua_pushnumber( L.L(), V.toFloat() );
						ArgCnt++;
						break;

					case QMetaType::Int:
						lua_pushinteger( L.L(), V.toInt() );
						ArgCnt++;
						break;

					case QMetaType::QString:
						lua_pushstring( L.L(), V.toString().toLatin1().constData() );
						ArgCnt++;
						break;

					default:
						if( V.typeName() == QStringLiteral( "lua_object::luaHandle" ) )
						{
							lua_object::luaHandle	LH = V.value<lua_object::luaHandle>();

							lua_object::lua_pushobjectid( L.L(), LH.O );

							ArgCnt++;
						}
						break;
				}
			}

			L.verbCall( V, ArgCnt );
		}

		mReadDone = true;
	} );

	connect( &mLineEdit, &LineEdit::dataOutput, [=]( const QByteArray &pData )
	{
		mConnection->write( pData );
	} );
}

bool InputSinkRead::input( const QString &pData )
{
	mLineEdit.dataInput( pData.toLatin1() );

	return( !mReadDone );
}

