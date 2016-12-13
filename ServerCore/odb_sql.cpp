#include "odb_sql.h"

#include "objectmanager.h"
#include "object.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlResult>

ODBSQL::ODBSQL()
{
	mDB = QSqlDatabase::addDatabase( "QSQLITE", "moo.sql" );

	if( !mDB.open() )
	{
		return;
	}
}

void ODBSQL::load()
{
	if( !mDB.isOpen() )
	{
		return;
	}
}

void ODBSQL::save()
{
	ObjectManager				&OM = *ObjectManager::instance();
	const ObjectManagerData		&Data = data( OM );

	if( !mDB.isOpen() )
	{
		return;
	}

	if( !mDB.tables().contains( "object" ) )
	{
		mDB.exec( "CREATE TABLE object ( "
				  "id INTEGER PRIMARY KEY,"
				  "parent INTEGER DEFAULT -1,"
				  "player BOOL DEFAULT false,"
				  "name TEXT,"
				  "owner INTEGER DEFAULT -1,"
				  "location INTEGER DEFAULT -1,"
				  "wizard BOOL DEFAULT false,"
				  "read BOOL DEFAULT true,"
				  "write BOOL DEFAULT false,"
				  "fertile BOOL DEFAULT false"
				  ")" );

		if( mDB.lastError().type() != QSqlError::NoError )
		{
			return;
		}
	}

	for( const Object *O : Data.mObjMap.values() )
	{
		saveObject( *O );
	}
}

Object *ODBSQL::object( ObjectId pIndex ) const
{
	Object		*O = newObject();
	ObjectData	&D = data( *O );

	QSqlQuery	Q;

	Q.prepare( "SELECT * FROM object WHERE id = :id" );

	Q.bindValue( ":id", pIndex );

	Q.exec();

	if( !Q.next() )
	{
		return( 0 );
	}

	D.mId = Q.value( "id" ).toInt();
	D.mLocation = Q.value( "location" ).toInt();
	D.mName = Q.value( "name" ).toString();
	D.mFertile = Q.value( "fertile" ).toBool();

	Q = mDB.exec( QString( "SELECT id FROM object WHERE parent = '%1'" ).arg( pIndex ) );

	while( Q.next() )
	{
		D.mChildren.append( Q.value( 0 ).toInt() );
	}

	Q = mDB.exec( QString( "SELECT id FROM object WHERE location = '%1'" ).arg( pIndex ) );

	while( Q.next() )
	{
		D.mContents.append( Q.value( 0 ).toInt() );
	}

	Q = mDB.exec( QString( "SELECT * FROM verb WHERE object = '%1'" ).arg( pIndex ) );

	Q = mDB.exec( QString( "SELECT * FROM property WHERE object = '%1'" ).arg( pIndex ) );

	return( 0 );
}

void bindObject( const ObjectData &D, QSqlQuery &Q )
{
	Q.bindValue( ":id", D.mId );
	Q.bindValue( ":parent", D.mParent );
	Q.bindValue( ":name", D.mName );
	Q.bindValue( ":player", D.mPlayer );
	Q.bindValue( ":owner", D.mPlayer );
}

void ODBSQL::registerObject( Object &pObject )
{
	const ObjectData	&D = data( pObject );

	QSqlQuery			 Q;

	Q.prepare( "INSERT INTO object ( id, parent, name, player, owner ) VALUES ( :id, :parent, :name, :player, :owner )" );

	bindObject( D, Q );

	Q.exec();
}

void ODBSQL::saveObject( const Object &O )
{
	const ObjectData	&D = data( O );

	QSqlQuery			 Q;

	Q.prepare( "UPDATE object SET ( parent = :parent, name = :name, player = :player, owner = :owner ) WHERE id = :id" );

	bindObject( D, Q );

	Q.exec();
}
