#ifndef OBJECT_H
#define OBJECT_H

#include "mooglobal.h"

#include <QString>
#include <QList>
#include <QVariant>
#include <QMap>
#include <QMultiMap>
#include <QDataStream>
#include <QVariant>
#include "verb.h"
#include "property.h"
#include "task.h"

class ObjectManager;

typedef struct ObjectData
{
	ObjectId			mId;

	// Fundamental Object Attributes

	bool				mPlayer;		// whether or not the object represents a player
	ObjectId			mParent;		// The object that is its parent
	QList<ObjectId>		mChildren;		// A list of the objects that are its children

	// Built-in Properties

	QString				mName;			// the usual name for this object
	QStringList			mAliases;
	ObjectId			mOwner;			// the player who controls access to it
	ObjectId			mLocation;		// where the object is in virtual reality
	QList<ObjectId>		mContents;		// a list of objects, the inverse of `location'
	bool				mProgrammer;	// does the object have programmer rights?
	bool				mWizard;		// does the object have wizard rights?
	bool				mRead;			// is the object publicly readable?
	bool				mWrite;			// is the object publicly writable?
	bool				mFertile;		// is the object fertile?
	bool				mRecycled;		//

	ConnectionId		mConnection;

	qint64				mLastRead;
	qint64				mLastUpdate;
	qint64				mLastWrite;

	// Properties and Verbs

	QMap<QString,Verb>			mVerbs;
	QMap<QString,Property>		mProperties;	// Properties defined on this object
	QList<Task>					mTasks;

	ObjectData( void )
		: mId( OBJECT_NONE ), mPlayer( false ), mParent( OBJECT_NONE ), mOwner( OBJECT_NONE ), mLocation( OBJECT_NONE ),
		  mProgrammer( false ), mWizard( false ), mRead( false ), mWrite( false ), mFertile( false ), mRecycled( false ),
		  mConnection( CONNECTION_NONE ), mLastRead( 0 ), mLastUpdate( 0 ), mLastWrite( 0 )
	{

	}
} ObjectData;

class Object
{
private:
	Object( void );

	virtual ~Object( void );

	friend class ObjectManager;
	friend class lua_object;
	friend class ODB;

public:

private:
	bool propFindRecurse( const QString &pName, Property **pProp, Object **pObject );
	void propDeleteRecurse( const QString &pName );

	bool verbFindRecurse( const QString &pName, Verb **pVerb, Object **pObject, ObjectId pDirectObjectId, const QString &pPreposition, ObjectId pIndirectObjectId );
	bool verbFindRecurse( const QString &pName, Verb **pVerb, Object **pObject );

	inline QMap<QString,Verb> &verbmap( void )
	{
		return( mData.mVerbs );
	}

	inline QMap<QString,Property> &propmap( void )
	{
		return( mData.mProperties );
	}

public:
	void move( Object *pWhere );

	void setParent( ObjectId pNewParentId );

	bool matchName( const QString &pName ) const;

	typedef enum Permissions
	{
		READ    = ( 1 << 0 ),
		WRITE   = ( 1 << 1 ),
		FERTILE = ( 1 << 2 )
	} Permissions;

	quint16 permissions( void ) const;
	void setPermissions( quint16 pPerms );

	void aliasAdd( const QString &pName );
	void aliasDelete( const QString &pName );

	//

	void ancestors( QList<ObjectId> &pList ) const;
	QVector<ObjectId> ancestors( void ) const;
	void descendants( QList<ObjectId> &pList ) const;
	QVector<ObjectId> descendants( void ) const;

	// Define property on this object
	void propAdd( const QString &pName, Property &pProp );
	void propDelete( const QString &pName );
	void propClear( const QString &pName );
	void propSet( const QString &pName, const QVariant &pValue );
	bool propFind( const QString &pName, Property **pProp, Object **pObject );
	void propNames( QStringList &pList ) const;

	const Property *prop( const QString &pName ) const;
	Property *prop( const QString &pName );
	Property *propParent( const QString &pName ) const;

	inline const QMap<QString,Property> &properties( void ) const
	{
		return( mData.mProperties );
	}

	void verbAdd( const QString &pName, Verb &pVerb );
	void verbDelete( const QString &pName );
	bool verbFind( const QString &pName, Verb **pVerb, Object **pObject, ObjectId pDirectObjectId, const QString &pPreposition, ObjectId pIndirectObjectId );
	bool verbFind( const QString &pName, Verb **pVerb, Object **pObject );

	Verb *verbMatch( const QString &pName, ObjectId pDirectObjectId, const QString &pPreposition, ObjectId pIndirectObjectId );
	Verb *verbParent( const QString &pName, ObjectId pDirectObjectId, const QString &pPreposition, ObjectId pIndirectObjectId ) const;

	Verb *verbMatch( const QString &pName );
	Verb *verbParent( const QString &pName ) const;

	inline const QMap<QString,Verb> &verbs( void ) const
	{
		return( mData.mVerbs );
	}

	const Verb *verb( const QString &pName ) const;
	Verb *verb( const QString &pName );

	//

	inline operator ObjectId ( void ) const
	{
		return( mData.mId );
	}

	inline ObjectId id( void ) const
	{
		return( mData.mId );
	}

	inline const QString &name( void ) const
	{
		return( mData.mName );
	}

	inline bool player( void ) const
	{
		return( mData.mPlayer );
	}

	inline ObjectId location( void ) const
	{
		return( mData.mLocation );
	}

	inline ObjectId parent( void ) const
	{
		return( mData.mParent );
	}

	inline const QList<ObjectId> &children( void ) const
	{
		return( mData.mChildren );
	}

	inline const QList<ObjectId> &contents( void ) const
	{
		return( mData.mContents );
	}

	inline bool valid( void ) const
	{
		return( !mData.mRecycled );
	}

	inline bool programmer( void ) const
	{
		return( mData.mProgrammer );
	}

	inline bool wizard( void ) const
	{
		return( mData.mWizard );
	}

	inline bool fertile( void ) const
	{
		return( mData.mFertile );
	}

	inline ObjectId owner( void ) const
	{
		return( mData.mOwner );
	}

	inline bool recycle( void ) const
	{
		return( mData.mRecycled );
	}

	inline bool read( void ) const
	{
		return( mData.mRead );
	}

	inline bool write( void ) const
	{
		return( mData.mWrite );
	}

	inline ConnectionId connection( void ) const
	{
		return( mData.mConnection );
	}

	inline const QStringList &aliases( void ) const
	{
		return( mData.mAliases );
	}

	void setOwner( ObjectId pOwner );

	void setPlayer( bool pPlayer );

	void setName( const QString &pName );

	void setProgrammer( bool pProgrammer );

	void setWizard( bool pWizard );

	void setRead( bool pRead );

	void setWrite( bool pWrite );

	void setFertile( bool pFertile );

	void setRecycled( bool pRecycle );

	void setConnection( ConnectionId pConnectionId );

protected:
	ObjectData &data( void )
	{
		return( mData );
	}

	const ObjectData &data( void ) const
	{
		return( mData );
	}

private:
	void setUpdated();

private:
	ObjectData			mData;
};

#endif // OBJECT_H
