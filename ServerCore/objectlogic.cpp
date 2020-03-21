#include <QDebug>

#include "objectlogic.h"
#include "objectmanager.h"
#include "object.h"
#include "verb.h"
#include "property.h"
#include <algorithm>
#include "mooexception.h"

#include "lua_moo.h"
#include "lua_object.h"
#include "lua_task.h"

ObjectId ObjectLogic::create( lua_task &pTask, ObjectId pUserId, ObjectId pParentId, ObjectId pOwnerId )
{
	Q_UNUSED( pTask )

	ObjectManager	&OM        = *ObjectManager::instance();
	Object			*objUser   = OM.object( pUserId );
	Object			*objParent = OM.object( pParentId );
	Object			*objOwner  = OM.object( pOwnerId );

	const bool		 UserIsValid    = ( pUserId == OBJECT_NONE || ( objUser != 0 && objUser->valid() ) );
	const bool		 ParentIsValid = ( objParent != 0 && objParent->valid() );
	const bool		 OwnerIsValid  = ( objOwner != 0 && objOwner->valid() );
	const bool		 UserIsWizard   = ( UserIsValid && ( objUser == 0 || objUser->wizard() ) );
//	const bool		 UserIsOwner = ( UserIsValid && OwnerIsValid && pUserId == objOwner->id() );
	const bool		 UserOwnsParent = ( UserIsValid && ParentIsValid && pUserId == objParent->owner() );

	if( !UserIsValid )
	{
		throw( mooException( E_ARGS, "user not found" ) );
	}

	if( pParentId != OBJECT_NONE && pParentId != OBJECT_UNSPECIFIED && !ParentIsValid )
	{
		throw( mooException( E_ARGS, "parent not found" ) );
	}

	if( pOwnerId != OBJECT_NONE && pOwnerId != OBJECT_UNSPECIFIED && !OwnerIsValid )
	{
		throw( mooException( E_ARGS, "owner not found" ) );
	}

	if( objParent != 0 && !objParent->fertile() && !UserOwnsParent && !UserIsWizard )
	{
		throw( mooException( E_PERM, "parent is not fertile" ) );
	}

	if( pOwnerId == OBJECT_UNSPECIFIED && pUserId != OBJECT_NONE )
	{
		pOwnerId = pUserId;
		objOwner = objUser;
	}

	if( pOwnerId != OBJECT_NONE && pOwnerId != OBJECT_UNSPECIFIED && pUserId != pOwnerId && !UserIsWizard )
	{
		throw( mooException( E_PERM, "" ) );
	}

	// The owner of the new object is either the programmer (if owner is not provided)
	// the new object itself (if owner was given as #-1)
	// or owner (otherwise).

	// If the intended owner of the new object has a property named `ownership_quota'
	//   and the value of that property is an integer, then create() treats that value
	//   as a quota.
	// If the quota is less than or equal to zero, then the quota is considered to be
	//   exhausted and create() raises E_QUOTA instead of creating an object.
	// Otherwise, the quota is decremented and stored back into the `ownership_quota'
	//   property as a part of the creation of the new object.

	Property			*Quota = nullptr;
	int					 QuotaValue = 0;

	if( objOwner )
	{
		Quota = objOwner->prop( "ownership_quota" );

		if( Quota && Quota->type() != QVariant::Int )
		{
			Quota = nullptr;
		}

		if( Quota )
		{
			QuotaValue = Quota->value().toInt();

			if( QuotaValue <= 0 )
			{
				throw( mooException( E_QUOTA, "" ) );
			}
		}
	}

	Object		*objNew = OM.newObject();

	if( !objNew )
	{
		throw( mooException( E_MEMORY, "" ) );
	}

	if( Quota )
	{
		Quota->setValue( int( QuotaValue - 1 ) );
	}

	if( objParent != 0 )
	{
		objNew->setParent( pParentId );
	}

	if( objOwner != 0 )
	{
		objNew->setOwner( pOwnerId );
	}

	if( pOwnerId == OBJECT_NONE )
	{
		objNew->setOwner( objNew->id() );
	}

	if( pOwnerId == OBJECT_UNSPECIFIED )
	{
		objNew->setOwner( OBJECT_NONE );
	}

	return( objNew->id() );
}

void ObjectLogic::chparent( lua_task &pTask, ObjectId pUserId, ObjectId pObjectId, ObjectId pNewParentId )
{
	Q_UNUSED( pTask )

	ObjectManager	&OM           = *ObjectManager::instance();

	Object			*objUser       = OM.object( pUserId );
	Object			*objObject     = OM.object( pObjectId );
	Object			*objOldParent = ( objObject != 0 ? OM.object( objObject->parent() ) : 0 );
	Object			*objNewParent = OM.object( pNewParentId );

	const bool		 UserIsValid    = ( pUserId == OBJECT_NONE || ( objUser != 0 && objUser->valid() ) );
	const bool		 ObjectIsValid  = ( objObject != 0 && objObject->valid() );
	const bool		 NewParentIsValid = ( objNewParent != 0 && objNewParent->valid() );
	const bool		 UserIsWizard   = ( UserIsValid && ( objUser == 0 || objUser->wizard() ) );
	const bool		 UserOwnsObject = ( UserIsValid && ObjectIsValid && pUserId == objObject->owner() );
	const bool		 UserOwnsNewParent = ( UserIsValid && NewParentIsValid && pUserId == objNewParent->owner() );

	// If object is not valid, or if new-parent is neither valid
	// nor equal to #-1, then E_INVARG is raised.

	if( !UserIsValid )
	{
		throw( mooException( E_INVARG, "user is not valid" ) );
	}

	if( !ObjectIsValid )
	{
		throw( mooException( E_INVARG, "object is not valid" ) );
	}

	if( pNewParentId != -1 && !NewParentIsValid )
	{
		throw( mooException( E_INVARG, "new parent is not valid" ) );
	}

	// If the programmer is neither a wizard or the owner of object,
	//   or if new-parent is not fertile (i.e., its `f' bit is not set)
	//   and the programmer is neither the owner of new-parent nor a
	//   wizard, then E_PERM is raised.

	if( !UserIsWizard || !UserOwnsObject || ( pNewParentId != -1 && !objNewParent->fertile() ) )
	{
		if( !UserOwnsNewParent && !UserIsWizard )
		{
			throw( mooException( E_PERM, "" ) );
		}
	}

	// If new-parent is equal to object or one of its current ancestors,
	//   E_RECMOVE is raised.

	if( objNewParent != 0 )
	{
		if( pNewParentId == objObject->id() )
		{
			throw( mooException( E_RECMOVE, "new-parent is equal to object" ) );
		}

		QList<ObjectId>			ObjectAncestors;

		objObject->ancestors( ObjectAncestors );

		if( ObjectAncestors.contains( pNewParentId ) )
		{
			throw( mooException( E_RECMOVE, "new-parent is equal to one of objects current ancestors" ) );
		}
	}

	// If object or one of its descendants defines a property with the
	//   same name as one defined either on new-parent or on one of its
	//   ancestors, then E_INVARG is raised.

	if( objNewParent != 0 )
	{
		// Get a list of the ObjectId's of NewParent and all its ancestors

		QList<ObjectId>		NewParAsc;

		objNewParent->ancestors( NewParAsc );

		NewParAsc.push_back( pNewParentId );

		std::sort( NewParAsc.begin(), NewParAsc.end() );
		std::unique( NewParAsc.begin(), NewParAsc.end() );

		// Get a list of all the defined properties for these objects

		QStringList		NewParPrp;

		for( ObjectId id : NewParAsc )
		{
			Object		*O = OM.object( id );

			if( O == 0 )
			{
				continue;
			}

			O->propNames( NewParPrp );
		}

		std::sort( NewParPrp.begin(), NewParPrp.end() );
		std::unique( NewParPrp.begin(), NewParPrp.end() );

		// Get a list of all the prop names on all the decendants of Object

		QList<ObjectId>		ObjDsc;

		objObject->descendants( ObjDsc );

		ObjDsc.push_front( pObjectId );

		QStringList			ObjDscPrp;

		for( ObjectId id : ObjDsc )
		{
			Object		*O = OM.object( id );

			if( O == 0 )
			{
				continue;
			}

			O->propNames( ObjDscPrp );
		}

		std::sort( ObjDscPrp.begin(), ObjDscPrp.end() );
		std::unique( ObjDscPrp.begin(), ObjDscPrp.end() );

		// Create an intersection between the two sets of prop names

		QList<QString>		IntPrp;

		std::set_intersection( NewParPrp.begin(), NewParPrp.end(), ObjDscPrp.begin(), ObjDscPrp.end(), std::back_inserter( IntPrp ) );

		// If there are any props shared, return an error

		if( !IntPrp.empty() )
		{
			throw( mooException( E_INVARG, "" ) );
		}
	}

	// Changing an object's parent can have the effect of removing some properties from and
	//   adding some other properties to that object and all of its descendants
	// All properties that are not removed or added in the reparenting
	//   process are completely unchanged.

	if( objOldParent != 0 && objNewParent != 0 )
	{
		// Let common be the nearest ancestor that object and new-parent have
		//   in common before the parent of object is changed.

		QList<ObjectId>		ObjAnc, NewAnc;
		ObjectId			NearestAncestor = -1;

		objObject->ancestors( ObjAnc );

		objNewParent->ancestors( NewAnc );

		NewAnc.push_front( pNewParentId );

		for( QList<ObjectId>::const_iterator i1 = ObjAnc.begin() ; NearestAncestor == -1 && i1 != ObjAnc.end() ; i1++ )
		{
			for( QList<ObjectId>::const_iterator i2 = NewAnc.begin() ; NearestAncestor == -1 && i2 != NewAnc.end() ; i2++ )
			{
				if( *i1 == *i2 )
				{
					NearestAncestor = *i1;
				}
			}
		}

		// Then all properties defined by ancestors of object under common
		//   (that is, those ancestors of object that are in turn descendants
		//   of common) are removed from object and all of its descendants.

		for( QList<ObjectId>::const_iterator it = ObjAnc.begin() ; *it != NearestAncestor && it != ObjAnc.end() ; it++ )
		{
			Object		*O = OM.object( *it );

			if( O == 0 )
			{
				continue;
			}

			QStringList		PrpLst;

			O->propNames( PrpLst );

			// TODO: Remove PrpLst props from all
		}

		// All properties defined by new-parent or its ancestors under common
		//   are added to object and all of its descendants.

		// As with create(), the newly-added properties are given the same
		//   permission bits as they have on new-parent, the owner of each
		//   added property is either the owner of the object it's added to
		//   (if the `c' permissions bit is set) or the owner of that property
		//   on new-parent, and the value of each added property is clear;
		//   see the description of the built-in function clear_property()
		//   for details.

		for( QList<ObjectId>::const_iterator it = NewAnc.begin() ; *it != NearestAncestor && it != NewAnc.end() ; it++ )
		{
			Object			*O = OM.object( *it );

			if( O == 0 )
			{
				continue;
			}

			QStringList		PrpLst;

			O->propNames( PrpLst );

			// TODO: Add PrpList to Object
		}
	}

	// If new-parent is equal to #-1, then object is given no parent at all;
	//   it becomes a new root of the parent/child hierarchy. In this case,
	//   all formerly inherited properties on object are simply removed.

	objObject->setParent( pNewParentId );
}

void ObjectLogic::recycle( lua_task &pTask, ObjectId pUserId, ObjectId pObjectId )
{
	ObjectManager	&OM           = *ObjectManager::instance();

	Object			*objUser       = OM.object( pUserId );
	Object			*objObject     = OM.object( pObjectId );
	Object			*objOwner      = ( objObject == 0 ? 0 : OM.object( objObject->owner() ) );
	Object			*objParent     = ( objObject == 0 ? 0 : OM.object( objObject->parent() ) );

	Verb			*V;

	const bool		 UserIsValid    = ( pUserId == OBJECT_NONE || ( objUser != 0 && objUser->valid() ) );
	const bool		 ObjectIsValid  = ( objObject != 0 && objObject->valid() );
	const bool		 UserIsWizard   = ( UserIsValid && ( objUser == 0 || objUser->wizard() ) );
	const bool		 UserOwnsObject = ( UserIsValid && ObjectIsValid && pUserId == objObject->owner() );

	// If object is not valid, then E_INVARG is raised

	if( !UserIsValid )
	{
		throw( mooException( E_INVARG, "user is not valid" ) );
	}

	if( !ObjectIsValid )
	{
		throw( mooException( E_INVARG, "object is not valid" ) );
	}

	// The programmer must either own object or be a wizard; otherwise, E_PERM is raised.

	if( !UserIsWizard && !UserOwnsObject )
	{
		throw( mooException( E_PERM, "programmer is not owner or wizard" ) );
	}

	// The children of object are reparented to the parent of object.

	ObjectId	idParent = ( objParent == 0 ? OBJECT_NONE : objParent->id() );

	objObject->setParent( OBJECT_NONE );

	QList<ObjectId>		Children = objObject->children();

	for( ObjectId idChild : Children )
	{
		chparent( pTask, pUserId, idChild, idParent );
	}

	// Before object is recycled, each object in its contents is moved to #-1 (implying a call to object's exitfunc verb, if any)

	QList<ObjectId>		Contents = objObject->contents();

	for( ObjectId idContents : Contents )
	{
		move( pTask, pUserId, idContents, OBJECT_NONE );
	}

	// and then object's `recycle' verb, if any, is called with no arguments.

	if( ( V = objObject->verbMatch( "recycle" ) ) != 0 )
	{
		pTask.verbCall( objObject->id(), V );
	}

	move( pTask, pUserId, pObjectId, OBJECT_NONE );

	// if the owner of the former object has a property named `ownership_quota' and the value of that property is a integer, then recycle() treats that value as a quota and increments it by one, storing the result back into the `ownership_quota' property.

	if( objOwner )
	{
		Property		*Quota = objOwner->prop( "ownership_quota" );

		if( Quota && Quota->type() == QVariant::Int )
		{
			int		QuotaValue = Quota->value().toInt();

			Quota->setValue( QuotaValue + 1 );
		}
	}

	OM.recycle( objObject );
}

void ObjectLogic::move( lua_task &pTask, ObjectId pUserId, ObjectId pObjectId, ObjectId pWhereId )
{
	ObjectManager	&OM           = *ObjectManager::instance();

	lua_State		*L             = pTask.L();

	Object			*objUser       = OM.object( pUserId );
	Object			*objObject     = OM.object( pObjectId );
	Object			*objWhere      = OM.object( pWhereId );
	Object			*objFrom       = ( objObject != 0 ? OM.object( objObject->location() ) : 0 );
	Verb			*FndVrb;
	Object			*FndObj;

	const bool		 UserIsValid    = ( pUserId == OBJECT_NONE || ( objUser != 0 && objUser->valid() ) );
	const bool		 ObjectIsValid  = ( objObject != 0 && objObject->valid() );
	const bool		 WhereIsValid   = ( objWhere != 0 && objWhere->valid() );
	const bool		 FromIsValid    = ( objFrom != 0 && objFrom->valid() );
	const bool		 UserIsWizard   = ( UserIsValid && ( objUser == 0 || objUser->wizard() ) );
	const bool		 UserOwnsObject = ( UserIsValid && ObjectIsValid && pUserId == objObject->owner() );

	// what should be a valid object
	// otherwise E_INVARG is raised.

	if( !ObjectIsValid )
	{
		throw( mooException( E_INVARG, "what is not a valid object" ) );
	}

	// where should be either a valid object or #-1
	//   (denoting a location of `nowhere')
	// otherwise E_INVARG is raised.

	if( pWhereId != -1 && !WhereIsValid )
	{
		throw( mooException( E_INVARG, "where should be either a valid object or #-1" ) );
	}

	// The programmer must be either the owner of what or a wizard;
	// otherwise, E_PERM is raised.

	if( !UserOwnsObject && !UserIsWizard )
	{
		throw( mooException( E_PERM, "The programmer must be either the owner of what or a wizard" ) );
	}

	// If where is a valid object, then the verb-call
	//
	//   where:accept(what)
	//
	// is performed before any movement takes place.
	// If the verb returns a false value and the programmer is not a wizard,
	//   then where is considered to have refused entrance to what;
	//   move() raises E_NACC.

	// If where does not define an accept verb, then it is treated as
	//   if it defined one that always returned false.

	if( objWhere != 0 && !UserIsWizard )
	{
		if( !objWhere->verbFind( "accept", &FndVrb, &FndObj ) )
		{
			throw( mooException( E_NACC, "where doesn't have an accept verb" ) );
		}

		lua_object::lua_pushobject( L, objObject );

		int		Results  = pTask.verbCall( objWhere->id(), FndVrb, 1 );
		bool	Accepted = ( Results == 1 && lua_isboolean( L, -1 ) && lua_toboolean( L, -1 ) );

		lua_pop( L, Results );

		if( !Accepted )
		{
			throw( mooException( E_NACC, "where would not accept object" ) );
		}
	}

	// If moving what into where would create a loop in the
	// containment hierarchy (i.e., what would contain itself,
	// even indirectly), then E_RECMOVE is raised instead.

	QList<ObjectId>		ObjDsc;

	objObject->descendants( ObjDsc );

	if( ObjDsc.contains( pWhereId ) )
	{
		throw( mooException( E_RECMOVE, "moving what into where would create a loop" ) );
	}

	// The `location' property of what is changed to be where,
	//   and the `contents' properties of the old and new locations
	//   are modified appropriately. Let old-where be the location
	//   of what before it was moved.

	objObject->move( objWhere );

	// If old-where is a valid object, then the verb-call
	//
	//   old-where:exitfunc(what,to)
	//
	// is performed and its result is ignored;
	// it is not an error if old-where does not define a verb named
	// `exitfunc'.

	if( FromIsValid && objFrom->verbFind( "exitfunc", &FndVrb, &FndObj ) )
	{
		lua_object::lua_pushobject( L, objObject );
		lua_object::lua_pushobjectid( L, objWhere ? objWhere->id() : OBJECT_NONE );

		int		Results  = pTask.verbCall( objFrom->id(), FndVrb, 2 );

		lua_pop( L, Results );
	}

	// Finally, if where and what are still valid objects,
	// and where is still the location of what, then the verb-call
	//
	//   where:enterfunc(what,from)
	//
	// is performed and its result is ignored; again, it is not an
	//   error if where does not define a verb named `enterfunc'.

	if( WhereIsValid && objWhere->verbFind( "enterfunc", &FndVrb, &FndObj ) )
	{
		lua_object::lua_pushobject( L, objObject );
		lua_object::lua_pushobjectid( L, objFrom ? objFrom->id() : OBJECT_NONE );

		int		Results  = pTask.verbCall( objWhere->id(), FndVrb, 2 );

		lua_pop( L, Results );
	}
}

