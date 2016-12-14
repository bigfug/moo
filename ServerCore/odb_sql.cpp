#include "odb_sql.h"

#include <QDebug>
#include <QBuffer>
#include <QDataStream>

#include "objectmanager.h"
#include "object.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlResult>

ODBSQL::ODBSQL()
{
	mDB = QSqlDatabase::addDatabase( "QSQLITE" );

	mDB.setDatabaseName( "moo.sql" );

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
				  "name VARCHAR(255),"
				  "player BOOLEAN DEFAULT false,"
				  "owner INTEGER DEFAULT -1,"
				  "location INTEGER DEFAULT -1,"
				  "programmer BOOLEAN DEFAULT false,"
				  "wizard BOOLEAN DEFAULT false,"
				  "read BOOLEAN DEFAULT true,"
				  "write BOOLEAN DEFAULT false,"
				  "fertile BOOLEAN DEFAULT false"
				  ")" );

		QSqlError		DBE = mDB.lastError();

		if( DBE.type() != QSqlError::NoError )
		{
			return;
		}
	}

	if( !mDB.tables().contains( "verb" ) )
	{
		mDB.exec( "CREATE TABLE verb ( "
				  "name VARCHAR(255),"
				  "object INTEGER,"
				  "owner INTEGER DEFAULT -1,"
				  "read BOOLEAN DEFAULT true,"
				  "write BOOLEAN DEFAULT false,"
				  "execute BOOLEAN DEFAULT false,"
				  "script TEXT,"
				  "dobj VARCHAR(20),"
				  "preptype VARCHAR(20),"
				  "iobj VARCHAR(20),"
				  "prep VARCHAR(255),"
				  "aliases VARCHAR(255)"
				  ")" );

		QSqlError		DBE = mDB.lastError();

		if( DBE.type() != QSqlError::NoError )
		{
			return;
		}
	}

	if( !mDB.tables().contains( "property" ) )
	{
		mDB.exec( "CREATE TABLE property ( "
				  "name VARCHAR(255),"
				  "object INTEGER,"
				  "parent INTEGER DEFAULT -1,"
				  "owner INTEGER DEFAULT -1,"
				  "read BOOLEAN DEFAULT true,"
				  "write BOOLEAN DEFAULT false,"
				  "change BOOLEAN DEFAULT false,"
				  "type VARCHAR(255),"
				  "value BLOB"
				  ")" );

		QSqlError		DBE = mDB.lastError();

		if( DBE.type() != QSqlError::NoError )
		{
			return;
		}
	}

	if( !mDB.tables().contains( "task" ) )
	{
		mDB.exec( "CREATE TABLE task ( "
				  "id INTEGER,"
				  "timestamp INTEGER,"
				  "command TEXT,"
				  "player INTEGER DEFAULT -1,"
				  "connection INTEGER DEFAULT -1"
				  ")" );

		QSqlError		DBE = mDB.lastError();

		if( DBE.type() != QSqlError::NoError )
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
	QSqlQuery	Q;

	Q.prepare( "SELECT * FROM object WHERE id = :id" );

	Q.bindValue( ":id", pIndex );

	if( !Q.exec() )
	{
		return( 0 );
	}

	if( !Q.next() )
	{
		return( 0 );
	}

	Object		*O = newObject();

	if( !O )
	{
		return( 0 );
	}

	ObjectData	&D = data( *O );

	D.mId = Q.value( "id" ).toInt();
	D.mLocation = Q.value( "location" ).toInt();
	D.mName = Q.value( "name" ).toString();
	D.mFertile = Q.value( "fertile" ).toBool();
	D.mOwner = Q.value( "owner" ).toInt();
	D.mParent = Q.value( "parent" ).toInt();
	D.mPlayer = Q.value( "player" ).toBool();
	D.mProgrammer = Q.value( "programmer" ).toBool();
	D.mRead = Q.value( "read" ).toBool();
	D.mWizard = Q.value( "wizard" ).toBool();
	D.mWrite = Q.value( "write" ).toBool();

	Q.prepare( "SELECT id FROM object WHERE parent = :id" );

	Q.bindValue( ":id", pIndex );

	if( Q.exec() )
	{
		while( Q.next() )
		{
			D.mChildren.append( Q.value( 0 ).toInt() );
		}
	}

	Q.prepare( "SELECT id FROM object WHERE location = :id" );

	Q.bindValue( ":id", pIndex );

	if( Q.exec() )
	{
		while( Q.next() )
		{
			D.mContents.append( Q.value( 0 ).toInt() );
		}
	}

	Q.prepare( "SELECT * FROM verb WHERE object = :id" );

	Q.bindValue( ":id", pIndex );

	if( Q.exec() )
	{
		while( Q.next() )
		{
			Verb		V;
			FuncData	&FD = funcdata( V );
			VerbData	&VD = verbdata( V );

			FD.mExecute = Q.value( "execute" ).toBool();
			FD.mObject  = Q.value( "object" ).toInt();
			FD.mOwner   = Q.value( "owner" ).toInt();
			FD.mRead    = Q.value( "read" ).toBool();
			FD.mScript  = Q.value( "script" ).toString();
			FD.mWrite   = Q.value( "write" ).toBool();

			VD.mAliases = Q.value( "aliases" ).toString();
			VD.mDirectObject = Verb::argobj_from( Q.value( "dobj" ).toString().toLatin1() );
			VD.mIndirectObject = Verb::argobj_from( Q.value( "iobj" ).toString().toLatin1() );
			VD.mPrepositionType = Verb::argobj_from( Q.value( "preptype" ).toString().toLatin1() );
			VD.mPreposition = Q.value( "prep" ).toString();

			D.mVerbs.insert( Q.value( "name" ).toString(), V );
		}
	}

	Q.prepare( "SELECT * FROM property WHERE object = :id" );

	Q.bindValue( ":id", pIndex );

	if( Q.exec() )
	{
		while( Q.next() )
		{
			Property		 P;
			PropertyData	&PD = data( P );

			PD.mChange = Q.value( "change" ).toBool();
			PD.mOwner  = Q.value( "owner" ).toInt();
			PD.mParent = Q.value( "parent" ).toInt();
			PD.mRead   = Q.value( "read" ).toBool();
			PD.mWrite  = Q.value( "write" ).toBool();

			QMetaType::Type	 PT = QMetaType::Type( QMetaType::type( Q.value( "type" ).toString().toLatin1() ) );

			switch( PT )
			{
				case QMetaType::QString:
					PD.mValue = Q.value( "value" ).toString();
					break;

				case QMetaType::Bool:
					PD.mValue = Q.value( "value" ).toBool();
					break;

				case QMetaType::Double:
					PD.mValue = Q.value( "value" ).toDouble();
					break;

				case QMetaType::Int:
					PD.mValue = Q.value( "value" ).toInt();
					break;

				case QMetaType::Float:
					PD.mValue = Q.value( "value" ).toFloat();
					break;

				case QMetaType::Long:
					PD.mValue = Q.value( "value" ).toInt();
					break;

				case QMetaType::LongLong:
					PD.mValue = Q.value( "value" ).toLongLong();
					break;

				default:
					{
						QByteArray		Buffer = QByteArray::fromBase64( Q.value( "value" ).toString().toLatin1() );

						QBuffer		ReadBuffer( &Buffer );

						ReadBuffer.open( QIODevice::ReadOnly );

						QDataStream	ReadStream( &ReadBuffer );

						ReadStream >> PD.mValue;
					}
					break;

			}

			D.mProperties.insert( Q.value( "name" ).toString(), P );
		}
	}

	return( 0 );
}

void bindObject( const ObjectData &D, QSqlQuery &Q )
{
	Q.bindValue( ":id", D.mId );
	Q.bindValue( ":parent", D.mParent );
	Q.bindValue( ":name", D.mName );
	Q.bindValue( ":player", D.mPlayer );
	Q.bindValue( ":owner", D.mOwner );
	Q.bindValue( ":location", D.mLocation );
	Q.bindValue( ":programmer", D.mProgrammer );
	Q.bindValue( ":wizard", D.mWizard );
	Q.bindValue( ":read", D.mRead );
	Q.bindValue( ":write", D.mWrite );
	Q.bindValue( ":fertile", D.mFertile );
}

void bindFunc( const FuncData &D, QSqlQuery &Q )
{
	Q.bindValue( ":owner", D.mOwner );
	Q.bindValue( ":read", D.mRead );
	Q.bindValue( ":write", D.mWrite );
	Q.bindValue( ":execute", D.mExecute );
	Q.bindValue( ":script", D.mScript );
}

void bindVerb( const VerbData &D, QSqlQuery &Q )
{
	Q.bindValue( ":dobj", Verb::argobj_name( D.mDirectObject) );
	Q.bindValue( ":iobj", Verb::argobj_name( D.mIndirectObject ) );
	Q.bindValue( ":preptype", Verb::argobj_name( D.mPrepositionType ) );
	Q.bindValue( ":prep", D.mPreposition );
	Q.bindValue( ":aliases", D.mAliases );
}

void bindProperty( const QString &pName, ObjectId pObjectId, const PropertyData &D, QSqlQuery &Q )
{
	Q.bindValue( ":name", pName );
	Q.bindValue( ":object", pObjectId );
	Q.bindValue( ":parent", D.mParent );
	Q.bindValue( ":owner", D.mOwner );
	Q.bindValue( ":read", D.mRead );
	Q.bindValue( ":write", D.mWrite );
	Q.bindValue( ":change", D.mChange );
	Q.bindValue( ":type", D.mValue.typeName() );

	switch( QMetaType::Type( D.mValue.type() ) )
	{
		case QMetaType::QString:
		case QMetaType::Char:
			Q.bindValue( ":value", D.mValue );
			break;

		case QMetaType::Bool:
		case QMetaType::Double:
		case QMetaType::Int:
		case QMetaType::Float:
		case QMetaType::Long:
		case QMetaType::LongLong:
			Q.bindValue( ":value", D.mValue.toString() );
			break;

		default:
			{
				QByteArray		Buffer;

				{
					QBuffer		WriteBuffer( &Buffer );

					WriteBuffer.open( QIODevice::WriteOnly );

					QDataStream	WriteStream( &WriteBuffer );

					WriteStream << D.mValue;
				}

				Q.bindValue( ":value", Buffer.toBase64() );
			}
			break;
	}
}

void ODBSQL::registerObject( const Object &pObject )
{
	const ObjectData	&D = data( pObject );

	QSqlQuery			 Q;

	Q.prepare( "INSERT OR IGNORE INTO object "
			   "( id, parent, name, player, owner, location, programmer, wizard, read, write, fertile ) "
			   "VALUES "
			   "( :id, :parent, :name, :player, :owner, :location, :programmer, :wizard, :read, :write, :fertile )" );

	bindObject( D, Q );

	if( Q.exec() )
	{
		if( !Q.lastInsertId().isValid() )
		{
			Q.prepare( "UPDATE object SET ( parent = :parent, name = :name, player = :player, owner = :owner, location = :location, programmer = :programmer, wizard = :wizard, read = :read, write = :write, fertile = :fertile ) WHERE id = :id" );

			bindObject( D, Q );

			Q.exec();
		}

		return;
	}

	QSqlError		DBE = Q.lastError();

	if( DBE.type() != QSqlError::NoError )
	{
		qDebug() << DBE.databaseText() << DBE.driverText();
	}
}

void ODBSQL::saveObject( const Object &O )
{
	const ObjectData	&D = data( O );

	QSqlQuery			 Q1, Q2;

	registerObject( O );

	Q1.prepare( "DELETE FROM property WHERE object = :object AND name = :name" );

	Q2.prepare( "INSERT INTO property "
				"( name, object, parent, owner, read, write, change, type, value )"
				"VALUES "
				"( :name, :object, :parent, :owner, :read, :write, :change, :type, :value )"
				);

	for( QMap<QString,Property>::const_iterator it = D.mProperties.constBegin() ; it != D.mProperties.end() ; it++ )
	{
		Q1.bindValue( ":object", D.mId );
		Q1.bindValue( ":name", it.key() );

		bindProperty( it.key(), D.mId, data( it.value() ), Q2 );

		Q1.exec();
		Q2.exec();
	}

	QSqlQuery			 Q3, Q4;

	Q3.prepare( "DELETE FROM verb WHERE object = :object AND name = :name" );

	Q4.prepare( "INSERT INTO verb "
				"( name, object, owner, read, write, execute, script, dobj, preptype, iobj, prep, aliases )"
				"VALUES "
				"( :name, :object, :owner, :read, :write, :execute, :script, :dobj, :preptype, :iobj, :prep, :aliases )"
				);

	for( QMap<QString,Verb>::const_iterator it = D.mVerbs.constBegin() ; it != D.mVerbs.constEnd() ; it++ )
	{
		Q3.bindValue( ":object", D.mId );
		Q3.bindValue( ":name", it.key() );

		Q4.bindValue( ":object", D.mId );
		Q4.bindValue( ":name", it.key() );

		bindFunc( funcdata( it.value() ), Q4 );
		bindVerb( verbdata( it.value() ), Q4 );

		Q3.exec();
		Q4.exec();
	}
}
