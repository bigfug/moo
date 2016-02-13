#include "object.h"
#include "objectmanager.h"
#include <algorithm>
#include "verb.h"
#include "mooexception.h"

Object::Object( void )
	: mParent( -1 ), mOwner( -1 )
{
	mId = -1;
	mPlayer = false;
	mLocation = -1;
	mProgrammer = false;
	mWizard = false;
	mRead = false;
	mWrite = false;
	mFertile = false;
	mRecycled = false;
}

Object::~Object( void )
{
}

void Object::save( QDataStream &pData ) const
{
	pData << mId;
	pData << mPlayer;
	pData << mParent;

	pData << (quint32)mChildren.size();

	foreach( ObjectId O, mChildren )
	{
		pData << O;
	}

	pData << mName;
	pData << mOwner;
	pData << mLocation;

	pData << (quint32)mContents.size();

	foreach( ObjectId O, mContents )
	{
		pData << O;
	}

	pData << mProgrammer;
	pData << mWizard;
	pData << mRead;
	pData << mWrite;
	pData << mFertile;

	pData << (quint32)mVerbs.size();

	foreach( const QString &str, mVerbs.keys() )
	{
		pData << str;

		mVerbs.value( str ).save( pData );
	}

	pData << (quint32)mProperties.size();

	foreach( const QString &str, mProperties.keys() )
	{
		pData << str;

		mProperties.value( str ).save( pData );
	}
}

void Object::load(QDataStream &pData)
{
	ObjectId			id;
	quint32				c;

	pData >> mId;
	pData >> mPlayer;
	pData >> mParent;

	pData >> c;

	for( quint32 i = 0 ; i < c ; i++ )
	{
		pData >> id;

		mChildren.push_back( id );
	}

	pData >> mName;
	pData >> mOwner;
	pData >> mLocation;

	pData >> c;

	for( quint32 i = 0 ; i < c ; i++ )
	{
		pData >> id;

		mContents.push_back( id );
	}

	pData >> mProgrammer;
	pData >> mWizard;
	pData >> mRead;
	pData >> mWrite;
	pData >> mFertile;

	pData >> c;

	QString		 n;
	Verb		 v;

	for( quint32 i = 0 ; i < c ; i++ )
	{
		pData >> n;

		v.initialise();

		v.load( pData );

		v.setObject( mId );

		mVerbs.insert( n, v );
	}

	pData >> c;

	Property	 p;

	for( quint32 i = 0 ; i < c ; i++ )
	{
		pData >> n;

		p.load( pData );

		mProperties[ n ] = p;
	}
}

void Object::verbAdd( const QString &pName, Verb &pVerb )
{
	mVerbs.insert( pName, pVerb );
}

void Object::verbDelete( const QString &pName )
{
	mVerbs.remove( pName );
}

Property * Object::propParent( const QString &pName )
{
	if( mParent == -1 )
	{
		return( 0 );
	}

	Object      *ParentObject = ObjectManager::instance()->object( mParent );

	if( ParentObject == 0 )
	{
		return( 0 );
	}

	Property        *P = ParentObject->prop( pName );

	if( P != 0 )
	{
		return( P );
	}

	return( ParentObject->propParent( pName ) );
}

Verb * Object::verbParent( const QString &pName, ObjectId pDirectObjectId, const QString &pPreposition, ObjectId pIndirectObjectId )
{
	if( mParent == -1 )
	{
		return( 0 );
	}

	Object      *ParentObject = ObjectManager::instance()->object( mParent );

	if( ParentObject == 0 )
	{
		return( 0 );
	}

	Verb		*V = ParentObject->verbMatch( pName, pDirectObjectId, pPreposition, pIndirectObjectId );

	if( V != 0 )
	{
		return( V );
	}

	return( ParentObject->verbParent( pName, pDirectObjectId, pPreposition, pIndirectObjectId ) );
}

void Object::propAdd( const QString &pName, Property &pProp )
{
	mProperties.insert( pName, pProp );
}

void Object::propDeleteRecurse(const QString &pName)
{
	ObjectManager	&OM = *ObjectManager::instance();

	mProperties.remove( pName );

	foreach( ObjectId id, mChildren )
	{
		Object	*O = OM.object( id );

		if( O != 0 )
		{
			O->propDeleteRecurse( pName );
		}
	}
}

void Object::propDelete( const QString &pName )
{
	QMap<QString,Property>::iterator	it = mProperties.find( pName );

	if( it == mProperties.end() )
	{
		return;
	}

	if( it.value().parent() != -1 )
	{
		mProperties.remove( pName );
	}
	else
	{
		propDeleteRecurse( pName );
	}
}

void Object::propClear( const QString &pName )
{
	QMap<QString,Property>::iterator	it = mProperties.find( pName );

	if( it == mProperties.end() )
	{
		return;
	}

	if( it.value().parent() != OBJECT_NONE )
	{
		mProperties.remove( pName );
	}
}

void Object::propSet( const QString &pName, const QVariant &pValue )
{
	Property			*P;

	if( ( P = prop( pName ) ) != 0 )
	{
		if( P->value().type() != pValue.type() )
		{
			return;
		}

		P->setValue( pValue );
	}
	else if( ( P = propParent( pName ) ) != 0 )
	{
		if( P->value().type() != pValue.type() )
		{
			return;
		}

		Property		C = *P;

		if( P->change() )
		{
			C.setOwner( id() );
		}

		C.setValue( pValue );

		mProperties.insert( pName, C );
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

	Object		*Parent = ObjectManager::o( mParent );

	if( Parent != 0 )
	{
		return( Parent->propFindRecurse( pName, pProp, pObject ) );
	}

	return( false );
}

void Object::ancestors( QList<ObjectId> &pList )
{
	ObjectManager	&OM = *ObjectManager::instance();
	Object			*PO = OM.object( mParent );

	while( PO != 0 )
	{
		pList.push_back( PO->id() );

		PO = OM.object( PO->parent() );
	}
}

void Object::descendants( QList<ObjectId> &pList )
{
	ObjectManager	&OM = *ObjectManager::instance();

	pList.append( mChildren );

	foreach( ObjectId id, mChildren )
	{
		Object	*O = OM.object( id );

		if( O != 0 )
		{
			O->descendants( pList );
		}
	}
}

void Object::move( Object *pWhere )
{
	Object		*From = ( mLocation == -1 ? 0 : ObjectManager::instance()->object( mLocation ) );

	if( From != 0 )
	{
		int	Count = From->mContents.removeAll( mId );

		Q_ASSERT( Count == 1 );
	}

	if( pWhere != 0 )
	{
		Q_ASSERT( pWhere->mContents.removeAll( mId ) == 0 );

		pWhere->mContents.push_back( mId );
	}

	mLocation = ( pWhere == 0 ? -1 : pWhere->id() );
}

void Object::setParent( ObjectId pNewParentId )
{
	if( mParent == pNewParentId )
	{
		return;
	}

	if( mParent != -1 )
	{
		Object		*O = ObjectManager::instance()->object( mParent );

		Q_ASSERT( O != 0 );

		if( O != 0 )
		{
			int c = O->mChildren.removeAll( mId );

			Q_ASSERT( c == 1 );
		}
	}
	else
	{
		ObjectManager::instance()->topRem( this );
	}

	if( pNewParentId != -1 )
	{
		Object		*O = ObjectManager::instance()->object( pNewParentId );

		Q_ASSERT( O != 0 );

		if( O != 0 )
		{
			Q_ASSERT( O->mChildren.removeAll( mId ) == 0 );

			O->mChildren.push_back( mId );
		}
	}
	else
	{
		ObjectManager::instance()->topAdd( this );
	}

	mParent = pNewParentId;
}

void Object::propNames( QStringList &pList )
{
	pList = mProperties.keys();
}

Property *Object::prop( const QString &pName )
{
	QMap<QString,Property>::iterator	it = mProperties.find( pName );

	if( it == mProperties.end() )
	{
		return( 0 );
	}

	return( &it.value() );
}

quint16 Object::permissions( void ) const
{
	quint16			P = 0;

	if( mRead    ) P |= READ;
	if( mWrite   ) P |= WRITE;
	if( mFertile ) P |= FERTILE;

	return( P );
}

void Object::setPermissions( quint16 pPerms )
{
	mRead    = ( pPerms & READ );
	mWrite   = ( pPerms & WRITE );
	mFertile = ( pPerms & FERTILE );
}

bool Object::matchName( const QString &pName )
{
	if( mName.startsWith( pName, Qt::CaseInsensitive ) )
	{
		return( true );
	}

	Property		*p = prop( "aliases" );

	if( p == 0 )
	{
		return( false );
	}

	if( p->value().type() == QVariant::String )
	{
		return( p->value().toString().startsWith( pName, Qt::CaseInsensitive ) );
	}

	if( p->value().type() == QVariant::List )
	{
		foreach( QVariant v, p->value().toList() )
		{
			if( v.type() == QVariant::String && v.toString().startsWith( pName, Qt::CaseInsensitive ) )
			{
				return( true );
			}
		}
	}

	return( false );
}

Verb * Object::verbMatch( const QString &pName, ObjectId DirectObjectId, const QString &pPreposition, ObjectId IndirectObjectId )
{
	for( QMap<QString,Verb>::iterator it = mVerbs.begin() ; it != mVerbs.end() ; it++ )
	{
		Verb				&v = it.value();

		if( !v.matchArgs( id(), DirectObjectId, pPreposition, IndirectObjectId ) )
		{
			continue;
		}

		if( pName.compare( it.key(), Qt::CaseInsensitive ) != 0 )
		{
			const QString		&a = v.aliases();

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

bool Object::verbFind( const QString &pName, Verb **pVerb, Object **pObject, ObjectId pDirectObjectId, const QString &pPreposition, ObjectId pIndirectObjectId )
{
	return( verbFindRecurse( pName, pVerb, pObject, pDirectObjectId, pPreposition, pIndirectObjectId ) );
}

Verb *Object::verb( const QString &pName )
{
	for( QMap<QString,Verb>::iterator it = mVerbs.begin() ; it != mVerbs.end() ; it++ )
	{
		if( pName.compare( it.key(), Qt::CaseInsensitive ) != 0 )
		{
			continue;
		}

		return( &it.value() );
	}

	return( 0 );
}

bool Object::verbFindRecurse( const QString &pName, Verb **pVerb, Object **pObject, ObjectId pDirectObjectId, const QString &pPreposition, ObjectId pIndirectObjectId )
{
	if( ( *pVerb = verbMatch( pName, pDirectObjectId, pPreposition, pIndirectObjectId ) ) != 0 )
	{
		*pObject = this;

		return( true );
	}

	Object	*P = ObjectManager::o( parent() );

	if( P == 0 )
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
