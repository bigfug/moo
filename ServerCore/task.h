#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QList>
#include <QStringList>

#include "mooglobal.h"
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "taskentry.h"

class Connection;

class Task
{
private:
	TaskId				 mId;					// the task id
	qint64				 mTimeStamp;			// when the task was created
	QString				 mCommand;				// the command
	ObjectId			 mPlayer;				// the player who typed the command
	ObjectId			 mObject;				// the object on which this verb was found
	ObjectId			 mCaller;				// the same as `player'
	QString				 mVerb;					// the first word of the command
	QString				 mArgStr;				// everything after the first word of the command
	QStringList			 mArgs;					// the words in `argstr'
	QString				 mDirectObjectName;		// the direct object string found during parsing
	ObjectId			 mDirectObjectId;		// the direct object value found during matching
	QString				 mIndirectObjectName;	// the indirect object string
	ObjectId			 mIndirectObjectId;		// the indirect object value
	QString				 mPreposition;			// the prepositional phrase found during parsing
	ObjectId			 mPermissions;			// the current object that defines the permissions
	ObjectId			 mVerbObject;			// the object that the current verb belongs to

	const static QList<QString>	 mPrepositionList;

	friend class lua_task;

public:
	Task( const QString &pCommand = "" );

	Task( const TaskEntry &pEntry );

	Task( Task &&t ) = default;

	Task( const Task &t ) = default;

	virtual ~Task( void ) {}

	Task &operator =( const Task &T );

	TaskId id( void ) const
	{
		return( mId );
	}

	ObjectId player( void ) const
	{
		return( mPlayer );
	}

	ObjectId object( void ) const
	{
		return( mObject );
	}

	ObjectId caller( void ) const
	{
		return( mCaller );
	}

	const QString &verb( void ) const
	{
		return( mVerb );
	}

	const QString argstr( void ) const
	{
		return( mArgStr );
	}

	const QStringList &args( void ) const
	{
		return( mArgs );
	}

	qint64 timestamp( void ) const
	{
		return( mTimeStamp );
	}

	void setId( TaskId pId )
	{
		mId = pId;
	}

	void setPlayer( ObjectId pPlayer )
	{
		mPlayer = pPlayer;
	}

	void setArgStr( const QString &pArgStr )
	{
		mArgStr = pArgStr;
	}

	void setArgs( const QStringList &pArgs )
	{
		mArgs = pArgs;
	}

	void setVerb( const QString &pVerb )
	{
		mVerb = pVerb;
	}

	void setCaller( ObjectId pId )
	{
		mCaller = pId;
	}

	void setObject( ObjectId pId )
	{
		mObject = pId;
	}

	void setTimeStamp( qint64 pTime )
	{
		mTimeStamp = pTime;
	}

	void setVerbObject( ObjectId pId )
	{
		mVerbObject = pId;
	}

	ObjectId verbObject( void ) const
	{
		return( mVerbObject );
	}

	const QString &command( void ) const
	{
		return( mCommand );
	}

	const QString &directObjectName( void ) const
	{
		return( mDirectObjectName );
	}

	ObjectId directObjectId( void ) const
	{
		return( mDirectObjectId );
	}

	const QString &indirectObjectName( void ) const
	{
		return( mIndirectObjectName );
	}

	ObjectId indirectObjectId( void ) const
	{
		return( mIndirectObjectId );
	}

	const QString &preposition( void ) const
	{
		return( mPreposition );
	}

	void setDirectObjectName( const QString &pName )
	{
		mDirectObjectName = pName;
	}

	void setDirectObjectId( ObjectId pId )
	{
		mDirectObjectId = pId;
	}

	void setIndirectObjectName( const QString &pName )
	{
		mIndirectObjectName = pName;
	}

	void setIndirectObjectId( ObjectId pId )
	{
		mIndirectObjectId = pId;
	}

	void setPreposition( const QString &pName )
	{
		mPreposition = pName;
	}

	ObjectId permissions( void ) const
	{
		return( mPermissions );
	}

	void setPermissions( ObjectId pObjectId )
	{
		mPermissions = pObjectId;
	}

	void findObject( const QString &pName, QList<ObjectId> &pId ) const;

private:
	int findPreposition( const QStringList &pWords );

	void getDirectAndIndirect( const QStringList &pWords, int pPrpIdx );
};

#endif // TASK_H
