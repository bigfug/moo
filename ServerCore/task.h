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
	ObjectId			 mProgrammerId;

	ObjectId			 mPassObject;
	ObjectId			 mPassVerbObject;
	QString				 mPassVerb;

	const static QList<QString>	 mPrepositionList;

	friend class lua_task;

public:
	Task( const QString &pCommand = "" );

	Task( const TaskEntry &pEntry );

	Task( Task &&t ) = default;

	Task( const Task &t ) = default;

	virtual ~Task( void );

	Task &operator =( const Task &T );

	inline TaskId id( void ) const
	{
		return( mId );
	}

	inline ObjectId player( void ) const
	{
		return( mPlayer );
	}

	inline ObjectId object( void ) const
	{
		return( mObject );
	}

	inline ObjectId caller( void ) const
	{
		return( mCaller );
	}

	inline const QString &verb( void ) const
	{
		return( mVerb );
	}

	inline const QString argstr( void ) const
	{
		return( mArgStr );
	}

	inline const QStringList &args( void ) const
	{
		return( mArgs );
	}

	inline qint64 timestamp( void ) const
	{
		return( mTimeStamp );
	}

	inline void setId( TaskId pId )
	{
		mId = pId;
	}

	inline void setPlayer( ObjectId pPlayer )
	{
		mPlayer = pPlayer;
	}

	inline void setArgStr( const QString &pArgStr )
	{
		mArgStr = pArgStr;
	}

	inline void setArgs( const QStringList &pArgs )
	{
		mArgs = pArgs;
	}

	inline void setVerb( const QString &pVerb )
	{
		mVerb = pVerb;
	}

	void setCaller( ObjectId pId )
	{
		mCaller = pId;
	}

	inline void setObject( ObjectId pId )
	{
		mObject = pId;
	}

	inline void setTimeStamp( qint64 pTime )
	{
		mTimeStamp = pTime;
	}

	inline const QString &command( void ) const
	{
		return( mCommand );
	}

	inline const QString &directObjectName( void ) const
	{
		return( mDirectObjectName );
	}

	inline ObjectId directObjectId( void ) const
	{
		return( mDirectObjectId );
	}

	inline const QString &indirectObjectName( void ) const
	{
		return( mIndirectObjectName );
	}

	inline ObjectId indirectObjectId( void ) const
	{
		return( mIndirectObjectId );
	}

	inline const QString &preposition( void ) const
	{
		return( mPreposition );
	}

	inline void setDirectObjectName( const QString &pName )
	{
		mDirectObjectName = pName;
	}

	inline void setDirectObjectId( ObjectId pId )
	{
		mDirectObjectId = pId;
	}

	inline void setIndirectObjectName( const QString &pName )
	{
		mIndirectObjectName = pName;
	}

	inline void setIndirectObjectId( ObjectId pId )
	{
		mIndirectObjectId = pId;
	}

	inline void setPreposition( const QString &pName )
	{
		mPreposition = pName;
	}

	inline ObjectId programmer( void ) const
	{
		return( mProgrammerId );
	}

	inline void setProgrammer( ObjectId pObjectId )
	{
		mProgrammerId = pObjectId;
	}

	inline ObjectId passObject( void ) const
	{
		return( mPassObject );
	}

	inline void setPassObject( ObjectId pObjectId )
	{
		mPassObject = pObjectId;
	}

	inline ObjectId passVerbObject( void ) const
	{
		return( mPassVerbObject );
	}

	inline void setPassVerbObject( ObjectId pObjectId )
	{
		mPassVerbObject = pObjectId;
	}

	inline QString passVerb( void ) const
	{
		return( mPassVerb );
	}

	inline void setPassVerb( const QString &pVerb )
	{
		mPassVerb = pVerb;
	}

	void findObject( const QString &pName, QList<ObjectId> &pId ) const;

private:
	int findPreposition( const QStringList &pWords );

	void getDirectAndIndirect( const QStringList &pWords, int pPrpIdx );
};

#endif // TASK_H
