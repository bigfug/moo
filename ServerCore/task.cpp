#include "task.h"
#include "lua_moo.h"
#include "objectmanager.h"
#include "lua_object.h"
#include <QDateTime>

const QList<QString>	 Task::mPrepositionList =
{
	"with/using",
	"at/to",
	"in front of",
	"in/inside/into",
	"on top of/on/onto/upon",
	"out of/from inside/from",
	"over",
	"through",
	"under/underneath/beneath",
	"behind",
	"beside",
	"for/about",
	"is",
	"as",
	"off/off of",
	"up"
};

//const char		*Task::mPrepositionList[] =
//{
//	"with/using",
//	"at/to",
//	"in front of",
//	"in/inside/into",
//	"on top of/on/onto/upon",
//	"out of/from inside/from",
//	"over",
//	"through",
//	"under/underneath/beneath",
//	"behind",
//	"beside",
//	"for/about",
//	"is",
//	"as",
//	"off/off of",
//	"up",
//	0
//};

Task::Task( const QString &pCommand ) : mCommand( pCommand )
{
	mProgrammerId = OBJECT_NONE;

	mTimeStamp = QDateTime::currentMSecsSinceEpoch();

	mPlayer = OBJECT_NONE;
	mObject = OBJECT_NONE;
	mCaller = OBJECT_NONE;

	mDirectObjectId   = OBJECT_NONE;
	mIndirectObjectId = OBJECT_NONE;

	mPassObject     = OBJECT_NONE;
	mPassVerbObject = OBJECT_NONE;
}

Task::Task( const TaskEntry &pEntry )
{
	mProgrammerId = pEntry.playerid();

	mId			= pEntry.id();
	mTimeStamp	= pEntry.timestamp();
	mCommand	= pEntry.command();

	mPlayer		= pEntry.playerid();
	mObject		= OBJECT_NONE;
	mCaller		= OBJECT_NONE;

	mDirectObjectId   = OBJECT_NONE;
	mIndirectObjectId = OBJECT_NONE;

	mPassObject     = OBJECT_NONE;
	mPassVerbObject = OBJECT_NONE;
}

Task::~Task( void )
{
}

void Task::findObject( const QString &pName, QList<ObjectId> &pId ) const
{
	if( pName.isEmpty() )
	{
		return;
	}

	if( pName.startsWith( "#" ) )
	{
		bool	ok;
		int		id = pName.mid( 1 ).toInt( &ok );

		if( ok && ObjectManager::instance()->object( id ) )
		{
			pId.append( id );

			return;
		}

		return;
	}

	if( pName.startsWith( "$" ) )
	{
		Object		*R = ObjectManager::o( 0 );

		if( R )
		{
			Property	*P = R->prop( pName.mid( 1 ) );

			if( P )
			{
				ObjectId			OID = OBJECT_NONE;

				if( P->value().type() == QVariant::Int )
				{
					OID = P->value().toInt();
				}
				else
				{
					lua_object::luaHandle H = P->value().value<lua_object::luaHandle>();

					OID = H.O;
				}

				pId.append( OID );
			}
		}

		return;
	}

	if( pName == "me" )
	{
		pId.append( player() );

		return;
	}

	if( pName == "here" )
	{
		Object			*Player;

		if( ( Player = ObjectManager::instance()->object( player() ) ) != Q_NULLPTR )
		{
			pId.append( Player->location() );
		}

		return;
	}

	// Otherwise, the server considers all of the objects whose location is either the player
	// (i.e., the objects the player is "holding", so to speak)

	Object			*Player;

	if( ( Player = ObjectManager::instance()->object( player() ) ) != Q_NULLPTR )
	{
		const QList<ObjectId>	&Contents = Player->contents();

		for( ObjectId id : Contents )
		{
			Object		*Object;

			if( ( Object = ObjectManager::instance()->object( id ) ) != Q_NULLPTR )
			{
				if( Object->matchName( pName ) )
				{
					pId.append( id );
				}
			}
		}

		// or the room the player is in (i.e., the objects in the same room as the player);

		// it will try to match the object string against the various names for these objects.

		Object			*Location;

		if( ( Location = ObjectManager::instance()->object( Player->location() ) ) != Q_NULLPTR )
		{
			if( Location->matchName( pName ) )
			{
				pId.append( Location->id() );
			}

			const QList<ObjectId>	&Contents = Location->contents();

			for( ObjectId id : Contents )
			{
				Object		*Object;

				if( ( Object = ObjectManager::instance()->object( id ) ) != Q_NULLPTR )
				{
					if( Object->matchName( pName ) )
					{
						pId.append( id );
					}
				}
			}
		}
	}
}

int Task::findPreposition( const QStringList &pWords )
{
	mPreposition.clear();

	for( int i = 0 ; i < pWords.size() && mPreposition.isEmpty() ; i++ )
	{
		const QString	CurWrd = pWords.at( i );

		for( QString s : mPrepositionList )
		{
			QStringList		PrpSet = s.split( '/' );

			if( PrpSet.contains( CurWrd ) )
			{
				mPreposition = CurWrd;

				return( i );
			}
		}
	}

	return( pWords.size() );
}

void Task::getDirectAndIndirect( const QStringList &pWords, int pPrpIdx )
{
	mDirectObjectName.clear();
	mIndirectObjectName.clear();

	for( int i = 0 ; i < pWords.size() ; i++ )
	{
		if( i < pPrpIdx )
		{
			if( !mDirectObjectName.isEmpty() )
			{
				mDirectObjectName.append( " " );
			}

			mDirectObjectName.append( pWords.at( i ) );
		}
		else if( i > pPrpIdx )
		{
			if( !mIndirectObjectName.isEmpty() )
			{
				mIndirectObjectName.append( " " );
			}

			mIndirectObjectName.append( pWords.at( i ) );
		}
	}
}
