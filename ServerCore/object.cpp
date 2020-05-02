#include "object.h"
#include "objectmanager.h"
#include <algorithm>
#include "verb.h"
#include "mooexception.h"

Object::Object( void )
{
	mData.mId = OBJECT_NONE;
	mData.mParent = OBJECT_NONE;
	mData.mOwner = OBJECT_NONE;
	mData.mPlayer = false;
	mData.mLocation = OBJECT_NONE;
	mData.mModule = OBJECT_NONE;
	mData.mProgrammer = false;
	mData.mWizard = false;
	mData.mRead = false;
	mData.mWrite = false;
	mData.mFertile = false;
	mData.mRecycled = false;

	mData.mConnection = CONNECTION_NONE;

	mData.mLastRead   = ObjectManager::timestamp();
	mData.mLastUpdate = 0;
	mData.mLastWrite = 0;
}

Object::~Object( void )
{
}


quint16 Object::permissions( void ) const
{
	quint16			P = 0;

	if( mData.mRead    ) P |= READ;
	if( mData.mWrite   ) P |= WRITE;
	if( mData.mFertile ) P |= FERTILE;

	return( P );
}

Object::MatchResult Object::matchName( const QString &pName ) const
{
	if( !mData.mName.compare( pName, Qt::CaseInsensitive ) )
	{
		return( Object::MATCH_EXACT );
	}

	if( mData.mName.startsWith( pName, Qt::CaseInsensitive ) )
	{
		return( Object::MATCH_PARTIAL );
	}

	const Object			*O = this;

	while( O )
	{
		for( const QString &S : O->aliases() )
		{
			if( !S.compare( pName, Qt::CaseInsensitive ) )
			{
				return( Object::MATCH_EXACT );
			}

			if( S.startsWith( pName, Qt::CaseInsensitive ) )
			{
				return( Object::MATCH_PARTIAL );
			}
		}

		O = ObjectManager::o( O->parent() );
	}

	return( Object::MATCH_NONE );
}

void Object::move( Object *pWhere )
{
	Object		*From = ( mData.mLocation == OBJECT_NONE ? nullptr : ObjectManager::instance()->object( mData.mLocation ) );

	if( From )
	{
		int	Count = From->mData.mContents.removeAll( mData.mId );

		Q_ASSERT( Count == 1 );
	}

	if( pWhere )
	{
		Q_ASSERT( pWhere->mData.mContents.removeAll( mData.mId ) == 0 );

		pWhere->mData.mContents.push_back( mData.mId );
	}

	mData.mLocation = ( !pWhere ? OBJECT_NONE : pWhere->id() );

	setUpdated();
}

void Object::setParent( ObjectId pNewParentId )
{
	if( mData.mParent == pNewParentId )
	{
		return;
	}

	if( mData.mParent != OBJECT_NONE )
	{
		Object		*O = ObjectManager::instance()->object( mData.mParent );

		Q_ASSERT( O );

		if( O )
		{
			int c = O->mData.mChildren.removeAll( mData.mId );

			Q_ASSERT( c == 1 );
		}
	}

	if( pNewParentId != OBJECT_NONE )
	{
		Object		*O = ObjectManager::instance()->object( pNewParentId );

		Q_ASSERT( O );

		if( O )
		{
			Q_ASSERT( O->mData.mChildren.removeAll( mData.mId ) == 0 );

			O->mData.mChildren.push_back( mData.mId );
		}
	}

	mData.mParent = pNewParentId;

	setUpdated();
}

//----------------------------------------------------------------------------
// Ancestors/Decendents

void Object::ancestors( QList<ObjectId> &pList ) const
{
	ObjectManager	&OM = *ObjectManager::instance();
	Object			*PO = OM.object( mData.mParent );

	while( PO )
	{
		pList.push_back( PO->id() );

		PO = OM.object( PO->parent() );
	}
}

QVector<ObjectId> Object::ancestors() const
{
	QVector<ObjectId> L;

	ObjectManager	&OM = *ObjectManager::instance();
	Object			*PO = OM.object( mData.mParent );

	while( PO )
	{
		L << PO->id();

		PO = OM.object( PO->parent() );
	}

	return( L );
}

void Object::descendants( QList<ObjectId> &pList ) const
{
	ObjectManager	&OM = *ObjectManager::instance();

	pList.append( mData.mChildren );

	for( ObjectId id : mData.mChildren )
	{
		Object	*O = OM.object( id );

		if( O )
		{
			O->descendants( pList );
		}
	}
}

QVector<ObjectId> Object::descendants() const
{
	ObjectManager	&OM = *ObjectManager::instance();

	QVector<ObjectId> L;

	L.append( mData.mChildren.toVector() );

	for( ObjectId id : mData.mChildren )
	{
		Object	*O = OM.object( id );

		if( O )
		{
			L << O->descendants();
		}
	}

	return( L );
}

//----------------------------------------------------------------------------
// Properties

Property * Object::propParent( const QString &pName ) const
{
	if( mData.mParent == OBJECT_NONE )
	{
		return( Q_NULLPTR );
	}

	Object      *ParentObject = ObjectManager::instance()->object( mData.mParent );

	if( !ParentObject )
	{
		return( Q_NULLPTR );
	}

	Property        *P = ParentObject->prop( pName );

	if( P )
	{
		return( P );
	}

	return( ParentObject->propParent( pName ) );
}

QVariant Object::propValue( const QString &pName ) const
{
	const Property		*P = prop( pName );

	if( P )
	{
		return( P->value() );
	}

	P = propParent( pName );

	if( P )
	{
		return( P->value() );
	}

	return( QVariant() );
}

void Object::propAdd( const QString &pName, Property &pProp )
{
	pProp.setObject( id() );
	pProp.setName( pName );

	mData.mProperties.insert( pName, pProp );

	ObjectManager::instance()->addProperty( this, pName );
}

void Object::propDeleteRecurse( const QString &pName )
{
	ObjectManager	&OM = *ObjectManager::instance();

	if( mData.mProperties.remove( pName ) )
	{
		ObjectManager::instance()->deleteProperty( this, pName );
	}

	for( ObjectId id : mData.mChildren )
	{
		Object	*O = OM.object( id );

		if( O )
		{
			O->propDeleteRecurse( pName );
		}
	}
}

void Object::propDelete( const QString &pName )
{
	QMap<QString,Property>::iterator	it = mData.mProperties.find( pName );

	if( it == mData.mProperties.end() )
	{
		return;
	}

	if( it.value().parent() != OBJECT_NONE )
	{
		if( mData.mProperties.remove( pName ) )
		{
			ObjectManager::instance()->deleteProperty( this, pName );
		}
	}
	else
	{
		propDeleteRecurse( pName );
	}
}

void Object::propClear( const QString &pName )
{
	QMap<QString,Property>::iterator	it = mData.mProperties.find( pName );

	if( it == mData.mProperties.end() )
	{
		return;
	}

	if( it.value().parent() != OBJECT_NONE )
	{
		if( mData.mProperties.remove( pName ) )
		{
			ObjectManager::instance()->deleteProperty( this, pName );
		}
	}
}

void Object::propSet( const QString &pName, const QVariant &pValue )
{
	Property			*P;

	if( ( P = prop( pName ) ) != 0 )
	{
		if( P->value().type() != QVariant::Invalid && P->value().type() != pValue.type() )
		{
			throw mooException( E_TYPE, QString( "property %1 is type %2 - trying to set type %3" ).arg( P->name() ).arg( P->value().typeName() ).arg( pValue.typeName() ) );
		}

		P->setValue( pValue );
	}
	else if( ( P = propParent( pName ) ) != 0 )
	{
		if( P->value().type() != QVariant::Invalid && P->value().type() != pValue.type() )
		{
			throw mooException( E_TYPE, QString( "property %1 is type %2 - trying to set type %3" ).arg( P->name() ).arg( P->value().typeName() ).arg( pValue.typeName() ) );
		}

		Property		C = *P;

		C.setParent( C.object() );
		C.setObject( id() );

		// If the `c' permissions bit is set, then the owner of the property
		// on the new object is the same as the owner of the new object itself;
		// otherwise, the owner of the property on the new object is the same
		// as that on parent.

		if( P->change() )
		{
			C.setOwner( owner() );
		}

		if( C.value() != pValue )
		{
			C.setValue( pValue );

			mData.mProperties.insert( pName, C );

			ObjectManager::instance()->addProperty( this, pName );
		}
	}
}

bool Object::propFind( const QString &pName, Property **pProp, Object **pObject )
{
	return( propFindRecurse( pName, pProp, pObject ) );
}

bool Object::propFindRecurse( const QString &pName, Property **pProp, Object **pObject )
{
	if( ( *pProp = prop( pName ) ) != 0 )
	{
		*pObject = this;

		return( true );
	}

	Object		*Parent = ObjectManager::o( mData.mParent );

	if( Parent )
	{
		return( Parent->propFindRecurse( pName, pProp, pObject ) );
	}

	return( false );
}

void Object::propNames( QStringList &pList ) const
{
	pList = mData.mProperties.keys();
}

void Object::propAdd( QString pName, QVariant pVariant, ObjectId pOwnerId )
{
	Property		P;

	P.initialise();

	P.setValue( pVariant );
	P.setOwner( pOwnerId );

	propAdd( pName, P );
}

const Property *Object::prop( const QString &pName ) const
{
	QMap<QString,Property>::const_iterator	it = mData.mProperties.find( pName );

	if( it == mData.mProperties.end() )
	{
		return( Q_NULLPTR );
	}

	return( &it.value() );
}

Property *Object::prop( const QString &pName )
{
	QMap<QString,Property>::iterator	it = mData.mProperties.find( pName );

	if( it == mData.mProperties.end() )
	{
		return( Q_NULLPTR );
	}

	return( &it.value() );
}

//----------------------------------------------------------------------------
// Aliases

void Object::aliasAdd(const QString &pName)
{
	int		l1 = mData.mAliases.size();

	mData.mAliases.removeAll( pName );

	mData.mAliases.append( pName );

	int		l2 = mData.mAliases.size();

	if( l1 != l2 )
	{
		setUpdated();
	}
}

void Object::aliasDelete(const QString &pName)
{
	int		l1 = mData.mAliases.size();

	mData.mAliases.removeAll( pName );

	int		l2 = mData.mAliases.size();

	if( l1 != l2 )
	{
		setUpdated();
	}
}

//----------------------------------------------------------------------------
// Verbs

void Object::verbAdd( const QString &pName, Verb &pVerb )
{
	pVerb.setObject( id() );
	pVerb.setName( pName );

	mData.mVerbs.insert( pName, pVerb );

	ObjectManager::instance()->addVerb( this, pName );
}

void Object::verbDelete( const QString &pName )
{
	mData.mVerbs.remove( pName );

	ObjectManager::instance()->deleteVerb( this, pName );
}

Verb * Object::verbParent( const QString &pName, ObjectId pDirectObjectId, const QString &pPreposition, ObjectId pIndirectObjectId ) const
{
	if( mData.mParent == OBJECT_NONE )
	{
		return( Q_NULLPTR );
	}

	Object      *ParentObject = ObjectManager::instance()->object( mData.mParent );

	if( !ParentObject )
	{
		return( Q_NULLPTR );
	}

	Verb		*V = ParentObject->verbMatch( pName, pDirectObjectId, pPreposition, pIndirectObjectId );

	if( V )
	{
		return( V );
	}

	return( ParentObject->verbParent( pName, pDirectObjectId, pPreposition, pIndirectObjectId ) );
}

Verb *Object::verbMatch( const QString &pName )
{
	for( QMap<QString,Verb>::iterator it = mData.mVerbs.begin() ; it != mData.mVerbs.end() ; it++ )
	{
		Verb				&v = it.value();

		if( !Verb::matchPattern( v.name(), pName ) )
		{
			const QStringList		&a = v.aliases();

			if( a.isEmpty() )
			{
				continue;
			}

			if( !Verb::matchName( a, pName ) )
			{
				continue;
			}
		}

		return( &it.value() );
	}

	return( 0 );
}

Verb *Object::verbParent( const QString &pName ) const
{
	if( mData.mParent == OBJECT_NONE )
	{
		return( Q_NULLPTR );
	}

	Object      *ParentObject = ObjectManager::instance()->object( mData.mParent );

	if( !ParentObject )
	{
		return( Q_NULLPTR );
	}

	Verb		*V = ParentObject->verbMatch( pName );

	if( V )
	{
		return( V );
	}

	return( ParentObject->verbParent( pName ) );
}

Verb *Object::verbMatch( const QString &pName, ObjectId DirectObjectId, const QString &pPreposition, ObjectId IndirectObjectId )
{
	for( QMap<QString,Verb>::iterator it = mData.mVerbs.begin() ; it != mData.mVerbs.end() ; it++ )
	{
		const Verb				&v = it.value();

		if( !v.matchArgs( id(), DirectObjectId, pPreposition, IndirectObjectId ) )
		{
			continue;
		}

		if( !Verb::matchPattern( v.name(), pName ) && !Verb::matchName( v.aliases(), pName ) )
		{
			continue;
		}

		return( &it.value() );
	}

	return( Q_NULLPTR );
}

bool Object::verbFind( const QString &pName, Verb **pVerb, Object **pObject, ObjectId pDirectObjectId, const QString &pPreposition, ObjectId pIndirectObjectId )
{
	return( verbFindRecurse( pName, pVerb, pObject, pDirectObjectId, pPreposition, pIndirectObjectId ) );
}

bool Object::verbFind( const QString &pName, Verb **pVerb, Object **pObject )
{
	return( verbFindRecurse( pName, pVerb, pObject ) );
}

const Verb *Object::verb( const QString &pName ) const
{
	for( QMap<QString,Verb>::const_iterator it = mData.mVerbs.begin() ; it != mData.mVerbs.end() ; it++ )
	{
		if( pName.compare( it.key(), Qt::CaseInsensitive ) != 0 )
		{
			continue;
		}

		return( &it.value() );
	}

	return( Q_NULLPTR );
}

Verb *Object::verb( const QString &pName )
{
	for( QMap<QString,Verb>::iterator it = mData.mVerbs.begin() ; it != mData.mVerbs.end() ; it++ )
	{
		if( pName.compare( it.key(), Qt::CaseInsensitive ) != 0 )
		{
			continue;
		}

		return( &it.value() );
	}

	return( Q_NULLPTR );
}


bool Object::verbFindRecurse( const QString &pName, Verb **pVerb, Object **pObject, ObjectId pDirectObjectId, const QString &pPreposition, ObjectId pIndirectObjectId )
{
	if( ( *pVerb = verbMatch( pName, pDirectObjectId, pPreposition, pIndirectObjectId ) ) != 0 )
	{
		*pObject = this;

		return( true );
	}

	Object	*P = ObjectManager::o( parent() );

	if( !P )
	{
		return( false );
	}

	if( pDirectObjectId == id() )
	{
		pDirectObjectId = P->id();
	}

	if( pIndirectObjectId == id() )
	{
		pIndirectObjectId = P->id();
	}

	return( P->verbFindRecurse( pName, pVerb, pObject, pDirectObjectId, pPreposition, pIndirectObjectId ) );
}

bool Object::verbFindRecurse( const QString &pName, Verb **pVerb, Object **pObject )
{
	if( ( *pVerb = verbMatch( pName ) ) != 0 )
	{
		*pObject = this;

		return( true );
	}

	if( parent() == OBJECT_NONE )
	{
		return( false );
	}

	Object	*P = ObjectManager::o( parent() );

	if( !P )
	{
		return( false );
	}

	return( P->verbFindRecurse( pName, pVerb, pObject ) );
}

//----------------------------------------------------------------------------
// Setting methods

void Object::setPermissions( quint16 pPerms )
{
	mData.mRead    = ( pPerms & READ );
	mData.mWrite   = ( pPerms & WRITE );
	mData.mFertile = ( pPerms & FERTILE );

	setUpdated();
}

void Object::setUpdated( void )
{
	mData.mLastUpdate = ObjectManager::timestamp();

	ObjectManager::instance()->updateObject( this );
}

void Object::setOwner( ObjectId pOwner )
{
	if( mData.mOwner != pOwner )
	{
		mData.mOwner = pOwner;

		setUpdated();
	}
}

void Object::setPlayer( bool pPlayer )
{
	if( mData.mPlayer != pPlayer )
	{
		mData.mPlayer = pPlayer;

		setUpdated();
	}
}

void Object::setName( const QString &pName )
{
	if( mData.mName != pName )
	{
		mData.mName = pName;

		setUpdated();
	}
}

void Object::setProgrammer( bool pProgrammer )
{
	if( mData.mProgrammer != pProgrammer )
	{
		mData.mProgrammer = pProgrammer;

		setUpdated();
	}
}

void Object::setWizard( bool pWizard )
{
	if( mData.mWizard != pWizard )
	{
		mData.mWizard = pWizard;

		setUpdated();
	}
}

void Object::setRead( bool pRead )
{
	if( mData.mRead != pRead )
	{
		mData.mRead = pRead;

		setUpdated();
	}
}

void Object::setWrite( bool pWrite )
{
	if( mData.mWrite != pWrite )
	{
		mData.mWrite = pWrite;

		setUpdated();
	}
}

void Object::setFertile( bool pFertile )
{
	if( mData.mFertile != pFertile )
	{
		mData.mFertile = pFertile;

		setUpdated();
	}
}

void Object::setRecycled( bool pRecycle )
{
	mData.mRecycled = pRecycle;
}

void Object::setConnection( ConnectionId pConnectionId)
{
	if( mData.mConnection != pConnectionId )
	{
		mData.mConnection = pConnectionId;

		setUpdated();
	}
}

void Object::setModule( ObjectId pObjectId )
{
	if( mData.mModule != pObjectId )
	{
		mData.mModule = pObjectId;

		setUpdated();
	}
}

//----------------------------------------------------------------------------
// Signal/Slot support

void Object::objectConnect( QString pSrcVrb, ObjectId pDstObj, QString pDstVrb )
{
	for( const SignalConnection &c : mData.mSignalConnections )
	{
		if( c.mSrcVrb == pSrcVrb && c.mDstObj == pDstObj && c.mDstVrb == pDstVrb )
		{
			return;
		}
	}

	SignalConnection		SC{ id(), pSrcVrb, pDstObj, pDstVrb };

	mData.mSignalConnections << SC;

	ObjectManager::instance()->addSignalConnection( SC );
}

void Object::objectDisconnect( QString pSrcVrb, ObjectId pDstObj, QString pDstVrb )
{
	for( QVector<SignalConnection>::iterator it = mData.mSignalConnections.begin() ; it != mData.mSignalConnections.end() ; )
	{
		if( pSrcVrb.isEmpty() || pSrcVrb == it->mSrcVrb )
		{
			if( pDstObj == OBJECT_NONE || pDstObj == it->mDstObj )
			{
				if( pDstVrb.isEmpty() || pDstVrb == it->mDstVrb )
				{
					ObjectManager::instance()->deleteSignalConnection( *it );

					it = mData.mSignalConnections.erase( it );
				}
			}
		}

		it++;
	}
}

QVector<QPair<ObjectId, QString> > Object::objectSignals( QString pSrcVrb )
{
	QVector<QPair<ObjectId,QString>>	SigMap;

	for( const SignalConnection &c : mData.mSignalConnections )
	{
		if( pSrcVrb == c.mSrcVrb )
		{
			SigMap << QPair<ObjectId,QString>( c.mDstObj, c.mDstVrb );
		}
	}

	return( SigMap );
}

bool operator ==(const SignalConnection &lhs, const SignalConnection &rhs)
{
	return( lhs.mSrcObj == rhs.mSrcObj && lhs.mSrcVrb == rhs.mSrcVrb &&
			lhs.mDstObj == rhs.mDstObj && lhs.mDstVrb == rhs.mDstVrb );
}
