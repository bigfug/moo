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

#ifdef USING_PHYSICS
class btCollisionShape;
class btRigidBody;
#endif

class Object
{
private:
	Object( void );

	virtual ~Object( void );

	friend class ObjectManager;
	friend class lua_object;

public:

private:
	ObjectId			mId;

	// Fundamental Object Attributes

	bool				mPlayer;		// whether or not the object represents a player
	ObjectId			mParent;		// The object that is its parent
	QList<ObjectId>		mChildren;		// A list of the objects that are its children

#ifdef USING_PHYSICS
	btCollisionShape	*mCollisionShape;
	btRigidBody			*mRigidBody;
#endif

	// Built-in Properties

	QString				mName;			// the usual name for this object
	ObjectId			mOwner;			// the player who controls access to it
	ObjectId			mLocation;		// where the object is in virtual reality
	QList<ObjectId>		mContents;		// a list of objects, the inverse of `location'
	bool				mProgrammer;	// does the object have programmer rights?
	bool				mWizard;		// does the object have wizard rights?
	bool				mRead;			// is the object publicly readable?
	bool				mWrite;			// is the object publicly writable?
	bool				mFertile;		// is the object fertile?
	bool				mRecycled;		//

	// Properties and Verbs

	QMap<QString,Verb>			mVerbs;
	QMap<QString,Property>		mProperties;	// Properties defined on this object
	QList<Task>					mTasks;

	bool propFindRecurse( const QString &pName, Property **pProp, Object **pObject );
	void propDeleteRecurse( const QString &pName );

	bool verbFindRecurse( const QString &pName, Verb **pVerb, Object **pObject, ObjectId pDirectObjectId, const QString &pPreposition, ObjectId pIndirectObjectId );

	inline QMap<QString,Verb> &verbmap( void )
	{
		return( mVerbs );
	}

	inline QMap<QString,Property> &propmap( void )
	{
		return( mProperties );
	}

public:
	void move( Object *pWhere );
	void setParent( ObjectId pNewParentId );

	void save( QDataStream &pData ) const;
	void load( QDataStream &pData );

	bool matchName( const QString &pName );

	typedef enum Permissions
	{
		READ    = ( 1 << 0 ),
		WRITE   = ( 1 << 1 ),
		FERTILE = ( 1 << 2 )
	} Permissions;

	quint16 permissions( void ) const;
	void setPermissions( quint16 pPerms );

	//

	void ancestors( QList<ObjectId> &pList );
	void descendants( QList<ObjectId> &pList );

	// Define property on this object
	void propAdd( const QString &pName, Property &pProp );
	void propDelete( const QString &pName );
	void propClear( const QString &pName );
	void propSet( const QString &pName, const QVariant &pValue );
	bool propFind( const QString &pName, Property **pProp, Object **pObject );
	void propNames( QStringList &pList );

	Property *prop( const QString &pName );
	Property *propParent( const QString &pName );

	inline const QMap<QString,Property> &properties( void ) const
	{
		return( mProperties );
	}

	void verbAdd( const QString &pName, Verb &pVerb );
	void verbDelete( const QString &pName );
	bool verbFind( const QString &pName, Verb **pVerb, Object **pObject, ObjectId pDirectObjectId = OBJECT_NONE, const QString &pPreposition = "", ObjectId pIndirectObjectId = OBJECT_NONE );

	Verb *verbMatch( const QString &pName, ObjectId pDirectObjectId = OBJECT_NONE, const QString &pPreposition = "", ObjectId pIndirectObjectId = OBJECT_NONE );
	Verb *verbParent( const QString &pName, ObjectId pDirectObjectId = OBJECT_NONE, const QString &pPreposition = "", ObjectId pIndirectObjectId = OBJECT_NONE );

	inline const QMap<QString,Verb> &verbs( void ) const
	{
		return( mVerbs );
	}

	Verb *verb( const QString &pName );

	//

	inline operator ObjectId ( void ) const
	{
		return( mId );
	}

	inline ObjectId id( void ) const
	{
		return( mId );
	}

	inline const QString &name( void ) const
	{
		return( mName );
	}

	inline bool player( void ) const
	{
		return( mPlayer );
	}

	inline ObjectId location( void ) const
	{
		return( mLocation );
	}

	inline ObjectId parent( void ) const
	{
		return( mParent );
	}

	inline const QList<ObjectId> &children( void ) const
	{
		return( mChildren );
	}

	inline const QList<ObjectId> &contents( void ) const
	{
		return( mContents );
	}

	inline bool valid( void ) const
	{
		return( !mRecycled );
	}

	inline bool programmer( void ) const
	{
		return( mProgrammer );
	}

	inline bool wizard( void ) const
	{
		return( mWizard );
	}

	inline bool fertile( void ) const
	{
		return( mFertile );
	}

	inline ObjectId owner( void ) const
	{
		return( mOwner );
	}

	inline bool recycle( void ) const
	{
		return( mRecycled );
	}

	inline bool read( void ) const
	{
		return( mRead );
	}

	inline bool write( void ) const
	{
		return( mWrite );
	}

	inline void setOwner( ObjectId pOwner )
	{
		mOwner = pOwner;
	}

	void setPlayer( bool pPlayer )
	{
		mPlayer = pPlayer;
	}

	inline void setName( const QString &pName )
	{
		mName = pName;
	}

	inline void setProgrammer( bool pProgrammer )
	{
		mProgrammer = pProgrammer;
	}

	inline void setWizard( bool pWizard )
	{
		mWizard = pWizard;
	}

	inline void setRead( bool pRead )
	{
		mRead = pRead;
	}

	inline void setWrite( bool pWrite )
	{
		mWrite = pWrite;
	}

	inline void setFertile( bool pFertile )
	{
		mFertile = pFertile;
	}

	inline void setRecycle( bool pRecycle )
	{
		mRecycled = pRecycle;
	}
};

#endif // OBJECT_H
