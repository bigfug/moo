#include <QMultiMap>

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

#include "changeset/objectsetlocation.h"
#include "changeset/objectsetparent.h"
#include "changeset/objectrecycle.h"
#include "changeset/objectcreate.h"

ObjectId ObjectLogic::create( lua_task &pTask, ObjectId pUserId, ObjectId pParentId, ObjectId pOwnerId )
{
	Q_UNUSED( pTask )

	ObjectManager	&OM        = *ObjectManager::instance();
	Object			*objUser   = OM.object( pUserId );
	Object			*objParent = OM.object( pParentId );
	Object			*objOwner  = OM.object( pOwnerId );

	const bool		 UserIsValid   = ( pUserId == OBJECT_NONE || ( objUser != 0 && objUser->valid() ) );
	const bool		 ParentIsValid = ( objParent != 0 && objParent->valid() );
	const bool		 OwnerIsValid  = ( objOwner != 0 && objOwner->valid() );
	const bool		 UserIsWizard  = pTask.isWizard();
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
		throw( mooException( E_PERM, "permissions is not owner or wizard" ) );
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

	Property			*Quota      = nullptr;
	int					 QuotaValue = 1;

	if( objOwner )
	{
		Quota = objOwner->prop( "ownership_quota" );

		if( Quota && Quota->type() != QVariant::Double )
		{
			Quota = nullptr;
		}

		if( Quota )
		{
			QuotaValue = Quota->value().toInt();
		}
	}

	if( QuotaValue <= 0 )
	{
		throw( mooException( E_QUOTA, "No object quota left" ) );
	}

	Object		*objNew = OM.newObject();

	if( !objNew )
	{
		throw( mooException( E_MEMORY, "" ) );
	}

	if( Quota )
	{
		objOwner->propSet( Quota->name(), double( QuotaValue - 1 ) );
	}

	if( objParent )
	{
		objNew->setParent( pParentId );
	}

	if( objOwner )
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

	pTask.changeAdd( new change::ObjectCreate( objNew->id() ) );

	return( objNew->id() );
}

void ObjectLogic::chparent( lua_task &pTask, ObjectId pObjectId, ObjectId pNewParentId )
{
	ObjectManager	&OM           = *ObjectManager::instance();

	Object			*objObject    = OM.object( pObjectId );
	Object			*objNewParent = OM.object( pNewParentId );

	const bool		 UserIsValid       = pTask.isPermValid();
	const bool		 ObjectIsValid     = ( objObject    && objObject->valid() );
	const bool		 NewParentIsValid  = ( objNewParent && objNewParent->valid() );
	const bool		 UserIsWizard      = pTask.isWizard();
	const bool		 UserOwnsObject    = pTask.isOwner( objObject );
	const bool		 UserOwnsNewParent = pTask.isOwner( objNewParent );

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

	if( pNewParentId != OBJECT_NONE && !NewParentIsValid )
	{
		throw( mooException( E_INVARG, "new parent is not valid" ) );
	}

	// If the programmer is neither a wizard or the owner of object,
	// or if new-parent is not fertile (i.e., its `f' bit is not set) and the programmer is neither the owner of new-parent nor a wizard,
	// then E_PERM is raised.

	if( NewParentIsValid && !objNewParent->fertile() )
	{
		if( !UserIsWizard && !UserOwnsObject )
		{
			throw( mooException( E_PERM, "programmer doesn't own new parent" ) );
		}
	}

	// If new-parent is equal to object or one of its current ancestors,
	//   E_RECMOVE is raised.

	if( objNewParent )
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

	if( objNewParent )
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

			if( O )
			{
				O->propNames( NewParPrp );
			}
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

			if( O )
			{
				O->propNames( ObjDscPrp );
			}
		}

		std::sort( ObjDscPrp.begin(), ObjDscPrp.end() );
		std::unique( ObjDscPrp.begin(), ObjDscPrp.end() );

		// Create an intersection between the two sets of prop names

		QList<QString>		IntPrp;

		std::set_intersection( NewParPrp.begin(), NewParPrp.end(), ObjDscPrp.begin(), ObjDscPrp.end(), std::back_inserter( IntPrp ) );

		// If there are any props shared, return an error

		if( !IntPrp.empty() )
		{
			throw( mooException( E_INVARG, "new parent has conflicting properties" ) );
		}
	}

#if 0
	// Changing an object's parent can have the effect of removing some properties from and
	//   adding some other properties to that object and all of its descendants
	// All properties that are not removed or added in the reparenting
	//   process are completely unchanged.

	if( objOldParent && objNewParent )
	{
		// Let common be the nearest ancestor that object and new-parent have
		//   in common before the parent of object is changed.

		QList<ObjectId>		ObjAnc, NewAnc;
		ObjectId			NearestAncestor = -1;

		objObject->ancestors( ObjAnc );

		objNewParent->ancestors( NewAnc );

		NewAnc.push_front( pNewParentId );

		for( QList<ObjectId>::const_iterator i1 = ObjAnc.begin() ; i1 != ObjAnc.end() && NearestAncestor == -1 ; i1++ )
		{
			for( QList<ObjectId>::const_iterator i2 = NewAnc.begin() ; i2 != NewAnc.end() && NearestAncestor == -1 ; i2++ )
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

		for( QList<ObjectId>::const_iterator it = ObjAnc.begin() ; it != ObjAnc.end() && *it != NearestAncestor ; it++ )
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

		for( QList<ObjectId>::const_iterator it = NewAnc.begin() ; it != NewAnc.end() && *it != NearestAncestor ; it++ )
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
#endif

	// If new-parent is equal to #-1, then object is given no parent at all;
	//   it becomes a new root of the parent/child hierarchy. In this case,
	//   all formerly inherited properties on object are simply removed.

	QMultiMap<ObjectId,Property>		PrpMap;

	if( pNewParentId == OBJECT_NONE )
	{
		QList<ObjectId>		ObjAnc;
		QList<ObjectId>		ObjLst;

		objObject->ancestors( ObjAnc );

		ObjLst << objObject->id();

		objObject->descendants( ObjLst );

		for( ObjectId OID : ObjLst )
		{
			Object				*O = ObjectManager::o( OID );

			for( QString PrpNam : O->properties().keys() )
			{
				Property		*P = O->prop( PrpNam );

				if( ObjAnc.contains( P->parent() ) )
				{
					PrpMap.insert( OID, *P );

					O->propDelete( PrpNam );
				}
			}
		}
	}

	pTask.changeAdd( new change::ObjectSetParent( objObject, pNewParentId, PrpMap ) );
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
	const bool		 UserIsWizard   = pTask.isWizard();
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

	if( objObject->parent() != OBJECT_NONE )
	{
		pTask.changeAdd( new change::ObjectSetParent( objObject, OBJECT_NONE, QMultiMap<ObjectId,Property>() ) );
	}

	QList<ObjectId>		Children = objObject->children();

	for( ObjectId idChild : Children )
	{
		chparent( pTask, idChild, idParent );
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
		pTask.verbCall( V, 0, objObject->id() );
	}

	move( pTask, pUserId, pObjectId, OBJECT_NONE );

	// if the owner of the former object has a property named `ownership_quota' and the value of that property is a integer, then recycle() treats that value as a quota and increments it by one, storing the result back into the `ownership_quota' property.

	if( objOwner )
	{
		Property		*Quota = objOwner->prop( "ownership_quota" );

		if( Quota && Quota->type() == QVariant::Double )
		{
			int		QuotaValue = Quota->value().toInt();

			objOwner->propSet( Quota->name(), double( QuotaValue + 1 ) );
		}
	}

	pTask.changeAdd( new change::ObjectRecycle( objObject->id() ) );
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
	const bool		 UserIsWizard   = pTask.isWizard();
	const bool		 UserOwnsObject = ( UserIsValid && ObjectIsValid && pUserId == objObject->owner() );

	// what should be a valid object
	// otherwise E_INVARG is raised.

	if( !ObjectIsValid )
	{
		throw( mooException( E_INVARG, "move: what is not a valid object" ) );
	}

	// where should be either a valid object or #-1
	//   (denoting a location of `nowhere')
	// otherwise E_INVARG is raised.

	if( pWhereId != OBJECT_NONE && !WhereIsValid )
	{
		throw( mooException( E_INVARG, "move: where should be either a valid object or O_NONE" ) );
	}

	// The programmer must be either the owner of what or a wizard;
	// otherwise, E_PERM is raised.

	if( !UserOwnsObject && !UserIsWizard )
	{
		throw( mooException( E_PERM, "move: The programmer must be either the owner of what or a wizard" ) );
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

	if( objWhere && !UserIsWizard )
	{
		if( !objWhere->verbFind( "accept", &FndVrb, &FndObj ) )
		{
			throw( mooException( E_NACC, "move: where doesn't have an accept verb" ) );
		}

		lua_object::lua_pushobject( L, objObject );

		int		Results  = pTask.verbCall( FndVrb, 1, objWhere->id() );
		bool	Accepted = ( Results == 1 && lua_isboolean( L, -1 ) && lua_toboolean( L, -1 ) );

		lua_pop( L, Results );

		if( !Accepted )
		{
			throw( mooException( E_NACC, "move: where would not accept object" ) );
		}
	}

	// If moving what into where would create a loop in the
	// containment hierarchy (i.e., what would contain itself,
	// even indirectly), then E_RECMOVE is raised instead.

	if( pWhereId != OBJECT_NONE )
	{
		QList<ObjectId>		ObjDsc;

		objObject->descendants( ObjDsc );

		if( ObjDsc.contains( pWhereId ) )
		{
			throw( mooException( E_RECMOVE, "move: moving what into where would create a loop" ) );
		}
	}

	// The `location' property of what is changed to be where,
	//   and the `contents' properties of the old and new locations
	//   are modified appropriately. Let old-where be the location
	//   of what before it was moved.

	pTask.changeAdd( new change::ObjectSetLocation( objObject, pWhereId ) );

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

		int		Results  = pTask.verbCall( FndVrb, 2, objFrom->id() );

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

		int		Results  = pTask.verbCall( FndVrb, 2, objWhere->id() );

		lua_pop( L, Results );
	}
}

