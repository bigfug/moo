#include "odb_sql.h"

#include <QDebug>
#include <QBuffer>
#include <QDataStream>
#include <QStringList>
#include <QDateTime>
#include <QJsonDocument>
#include "lua_object.h"

#include "objectmanager.h"
#include "object.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlResult>

ODBSQL::ODBSQL()
{
	mDB = QSqlDatabase::addDatabase( "QSQLITE" );

	mDB.setDatabaseName( "moo.db" );

	if( !mDB.open() )
	{
		return;
	}

	if( !mDB.tables().contains( "object" ) )
	{
		mDB.exec( "CREATE TABLE object ( "
				  "id INTEGER PRIMARY KEY,"
				  "name VARCHAR(255),"
				  "aliases TEXT,"
				  "parent INTEGER DEFAULT -1,"
				  "player BOOLEAN DEFAULT false,"
				  "connection INTEGER DEFAULT -1,"
				  "owner INTEGER DEFAULT -1,"
				  "location INTEGER DEFAULT -1,"
				  "programmer BOOLEAN DEFAULT false,"
				  "wizard BOOLEAN DEFAULT false,"
				  "read BOOLEAN DEFAULT true,"
				  "write BOOLEAN DEFAULT false,"
				  "fertile BOOLEAN DEFAULT false"
				  "recycled BOOLEAN DEFAULT false"
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
				  "aliases TEXT,"
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
				  "code BLOB,"
				  "FOREIGN KEY(`object`) REFERENCES `object`(`id`)"
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
				  "value BLOB,"
				  "FOREIGN KEY(`object`) REFERENCES `object`(`id`)"
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
}

void ODBSQL::load()
{
	ObjectManager				&OM = *ObjectManager::instance();
	ObjectManagerData			&Data = data( OM );

	if( !mDB.isOpen() )
	{
		return;
	}

	QSqlQuery	Q1 = mDB.exec( "SELECT MAX( id ) FROM object" );

	if( Q1.next() )
	{
		Data.mObjNum = Q1.value( 0 ).toInt() + 1;
	}

	ObjectManager::instance()->recordRead();

	// Load all the player objects that were previously connected

	QSqlQuery	Q2 = mDB.exec( "SELECT id FROM object WHERE player AND connection != -1 AND !recycled" );

	while( Q2.next() )
	{
		ObjectManager::instance()->recordRead();

		ObjectManager::instance()->object( Q2.value( 0 ).toInt() );
	}
}

void ODBSQL::save()
{
//	ObjectManager				&OM = *ObjectManager::instance();
//	const ObjectManagerData		&Data = data( OM );

//	if( !mDB.isOpen() )
//	{
//		return;
//	}
}

void stringsToObjects( QVariantMap &PrpDat )
{
	for( QVariantMap::iterator it = PrpDat.begin() ; it != PrpDat.end() ; it++ )
	{
		if( QMetaType::Type( it.value().type() ) == QMetaType::QString )
		{
			QString					ST = it.value().toString();

			if( ST.startsWith( "##" ) )
			{
				ST.remove( 0, 1 );

				it.value() = ST;
			}
			else if( ST.startsWith( '#' ) )
			{
				ST.remove( 0, 1 );

				lua_object::luaHandle	LH;

				LH.O = ST.toInt();

				it.value() = QVariant::fromValue( LH );
			}
		}
		else if( QMetaType::Type( it.value().type() ) == QMetaType::QVariantMap )
		{
			QVariantMap		VM = it.value().toMap();

			stringsToObjects( VM );

			it.value() = VM;
		}
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

	ObjectManager::instance()->recordRead();

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
	D.mAliases = Q.value( "aliases" ).toString().split( ',', QString::SkipEmptyParts );
	D.mFertile = Q.value( "fertile" ).toBool();
	D.mOwner = Q.value( "owner" ).toInt();
	D.mParent = Q.value( "parent" ).toInt();
	D.mPlayer = Q.value( "player" ).toBool();
	D.mConnection = Q.value( "connection" ).toInt();
	D.mProgrammer = Q.value( "programmer" ).toBool();
	D.mRead = Q.value( "read" ).toBool();
	D.mWizard = Q.value( "wizard" ).toBool();
	D.mWrite = Q.value( "write" ).toBool();
	D.mRecycled = Q.value( "recycled" ).toBool();

	D.mLastRead = ObjectManager::timestamp();

	Q.prepare( "SELECT id FROM object WHERE parent = :id" );

	Q.bindValue( ":id", pIndex );

	if( Q.exec() )
	{
		while( Q.next() )
		{
			D.mChildren.append( Q.value( 0 ).toInt() );
		}
	}

	ObjectManager::instance()->recordRead();

	Q.prepare( "SELECT id FROM object WHERE location = :id" );

	Q.bindValue( ":id", pIndex );

	if( Q.exec() )
	{
		while( Q.next() )
		{
			D.mContents.append( Q.value( 0 ).toInt() );
		}
	}

	ObjectManager::instance()->recordRead();

	Q.prepare( "SELECT * FROM verb WHERE object = :id" );

	Q.bindValue( ":id", pIndex );

	if( Q.exec() )
	{
		while( Q.next() )
		{
			Verb		V;
			FuncData	&FD = funcdata( V );
			VerbData	&VD = verbdata( V );

			V.setObject( O->id() );
			V.setName( Q.value( "name" ).toString() );

			FD.mExecute = Q.value( "execute" ).toBool();
			FD.mObject  = Q.value( "object" ).toInt();
			FD.mOwner   = Q.value( "owner" ).toInt();
			FD.mRead    = Q.value( "read" ).toBool();
			FD.mScript  = Q.value( "script" ).toString();
			FD.mWrite   = Q.value( "write" ).toBool();

//			FD.mCompiled = Q.value( "code" ).toByteArray();
//			FD.mDirty    = FD.mCompiled.isEmpty();

			VD.mAliases = Q.value( "aliases" ).toString().split( ',', QString::SkipEmptyParts );
			VD.mDirectObject = Verb::argobj_from( Q.value( "dobj" ).toString().toLatin1() );
			VD.mIndirectObject = Verb::argobj_from( Q.value( "iobj" ).toString().toLatin1() );
			VD.mPrepositionType = Verb::argobj_from( Q.value( "preptype" ).toString().toLatin1() );
			VD.mPreposition = Q.value( "prep" ).toString();

//			if( FD.mDirty && !V.compile() )
//			{
//				FD.mDirty = false;

//				ObjectManager::instance()->updateVerb( O, FD.mName );
//			}

			D.mVerbs.insert( V.name(), V );
		}
	}

	ObjectManager::instance()->recordRead();

	Q.prepare( "SELECT * FROM property WHERE object = :id" );

	Q.bindValue( ":id", pIndex );

	if( Q.exec() )
	{
		while( Q.next() )
		{
			Property		 P;
			PropertyData	&PD = data( P );

			P.setObject( O->id() );
			P.setName( Q.value( "name" ).toString() );

			PD.mChange = Q.value( "change" ).toBool();
			PD.mOwner  = Q.value( "owner" ).toInt();
			PD.mParent = Q.value( "parent" ).toInt();
			PD.mRead   = Q.value( "read" ).toBool();
			PD.mWrite  = Q.value( "write" ).toBool();

			QString			 PropType = Q.value( "type" ).toString();

			if( PropType == "object" )
			{
				lua_object::luaHandle		LH;

				LH.O = Q.value( "value" ).toInt();

				PD.mValue = QVariant::fromValue( LH );
			}
			else if( PropType == "json" )
			{
				QJsonDocument		JSON = QJsonDocument::fromJson( Q.value( "value" ).toByteArray() );

				PD.mValue = JSON.toVariant();

				if( QMetaType::Type( PD.mValue.type() ) == QMetaType::QVariantMap )
				{
					QVariantMap		VM = PD.mValue.toMap();

					stringsToObjects( VM );

					PD.mValue = VM;
				}
			}
			else
			{
				QMetaType::Type	 PT = QMetaType::Type( QMetaType::type( PropType.toLatin1() ) );

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
			}

			D.mProperties.insert( P.name(), P );
		}
	}

	ObjectManager::instance()->recordRead();

	return( O );
}

bool ODBSQL::hasObject( ObjectId pIndex ) const
{
	QSqlQuery	Q;

	Q.prepare( "SELECT 1 FROM object WHERE id = :id" );

	Q.bindValue( ":id", pIndex );

	if( !Q.exec() )
	{
		return( true );
	}

	ObjectManager::instance()->recordRead();

	if( !Q.next() )
	{
		return( false );
	}

	return( true );
}

void bindObject( const ObjectData &D, QSqlQuery &Q )
{
	Q.bindValue( ":id", D.mId );
	Q.bindValue( ":parent", D.mParent );
	Q.bindValue( ":name", D.mName );
	Q.bindValue( ":aliases", D.mAliases.join( ',' ) );
	Q.bindValue( ":player", D.mPlayer );
	Q.bindValue( ":connection", D.mConnection );
	Q.bindValue( ":owner", D.mOwner );
	Q.bindValue( ":location", D.mLocation );
	Q.bindValue( ":programmer", D.mProgrammer );
	Q.bindValue( ":wizard", D.mWizard );
	Q.bindValue( ":read", D.mRead );
	Q.bindValue( ":write", D.mWrite );
	Q.bindValue( ":fertile", D.mFertile );
	Q.bindValue( ":recycled", D.mRecycled );
}

void bindFunc( const FuncData &D, QSqlQuery &Q )
{
	Q.bindValue( ":object", D.mObject );
	Q.bindValue( ":name",   D.mName );
	Q.bindValue( ":owner", D.mOwner );
	Q.bindValue( ":read", D.mRead );
	Q.bindValue( ":write", D.mWrite );
	Q.bindValue( ":execute", D.mExecute );
	Q.bindValue( ":script", D.mScript );
	Q.bindValue( ":code", D.mCompiled );
}

void bindVerb( const VerbData &D, QSqlQuery &Q )
{
	Q.bindValue( ":dobj", Verb::argobj_name( D.mDirectObject) );
	Q.bindValue( ":iobj", Verb::argobj_name( D.mIndirectObject ) );
	Q.bindValue( ":preptype", Verb::argobj_name( D.mPrepositionType ) );
	Q.bindValue( ":prep", D.mPreposition );
	Q.bindValue( ":aliases", D.mAliases.join( ',' ) );
}

void objectsToStrings( QVariantMap &PrpDat )
{
	for( QVariantMap::iterator it = PrpDat.begin() ; it != PrpDat.end() ; it++ )
	{
		if( it.value().canConvert<lua_object::luaHandle>() )
		{
			lua_object::luaHandle	LH = it.value().value<lua_object::luaHandle>();

			it.value() = QString( "#%1" ).arg( LH.O );
		}
		else if( QMetaType::Type( it.value().type() ) == QMetaType::QString )
		{
			QString					ST = it.value().toString();

			if( ST.startsWith( '#' ) )
			{
				ST.prepend( '#' );

				it.value() = ST;
			}
		}
		else if( QMetaType::Type( it.value().type() ) == QMetaType::QVariantMap )
		{
			QVariantMap		VM = it.value().toMap();

			objectsToStrings( VM );

			it.value() = VM;
		}
	}
}

void bindProperty( const PropertyData &D, QSqlQuery &Q )
{
	Q.bindValue( ":name", D.mName );
	Q.bindValue( ":object", D.mObject );
	Q.bindValue( ":parent", D.mParent );
	Q.bindValue( ":owner", D.mOwner );
	Q.bindValue( ":read", D.mRead );
	Q.bindValue( ":write", D.mWrite );
	Q.bindValue( ":change", D.mChange );

	if( D.mValue.canConvert<lua_object::luaHandle>() )
	{
		lua_object::luaHandle	LH = D.mValue.value<lua_object::luaHandle>();

		Q.bindValue( ":type", "object" );
		Q.bindValue( ":value", LH.O );
	}
	else
	{
		switch( QMetaType::Type( D.mValue.type() ) )
		{
			case QMetaType::QString:
			case QMetaType::Char:
			case QMetaType::UChar:
				Q.bindValue( ":type", D.mValue.typeName() );
				Q.bindValue( ":value", D.mValue );
				break;

			case QMetaType::Bool:
			case QMetaType::Double:
			case QMetaType::Int:
			case QMetaType::Float:
			case QMetaType::Long:
			case QMetaType::LongLong:
			case QMetaType::UInt:
			case QMetaType::ULong:
			case QMetaType::ULongLong:
			case QMetaType::UShort:
			case QMetaType::Short:
				Q.bindValue( ":type", D.mValue.typeName() );
				Q.bindValue( ":value", D.mValue.toString() );
				break;

			default:
#if 0
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
#else
				{
					Q.bindValue( ":type", "json" );

					QVariantMap		PrpDat = D.mValue.toMap();

					objectsToStrings( PrpDat );

					QJsonDocument	Json = QJsonDocument::fromVariant( PrpDat );

					Q.bindValue( ":value", Json.toJson( QJsonDocument::Compact ) );

					qDebug() << Json.toJson( QJsonDocument::Compact );
				}
#endif
				break;
		}
	}
}

void ODBSQL::addObject( Object &pObject )
{
	ObjectData	&D = data( pObject );

	QSqlQuery			 Q;

	Q.prepare( "INSERT INTO object "
			   "( id, parent, name, aliases, player, connection, owner, location, programmer, wizard, read, write, fertile, recycled ) "
			   "VALUES "
			   "( :id, :parent, :name, :aliases, :player, :connection, :owner, :location, :programmer, :wizard, :read, :write, :fertile, :recycled )" );

	bindObject( D, Q );

	Q.exec();

	ObjectManager::instance()->recordWrite();

	QSqlError		DBE = Q.lastError();

	if( DBE.type() != QSqlError::NoError )
	{
		qDebug() << DBE.databaseText() << DBE.driverText();
	}

	D.mLastWrite = ObjectManager::timestamp();
}

void ODBSQL::deleteObject( Object &pObject )
{
	ObjectData	&D = data( pObject );

	QSqlQuery		Q;

	Q.prepare( "UPDATE object SET recycled = TRUE WHERE id = :id" );

	bindObject( D, Q );

	Q.exec();

	ObjectManager::instance()->recordWrite();

	QSqlError		DBE = Q.lastError();

	if( DBE.type() != QSqlError::NoError )
	{
		qDebug() << DBE.databaseText() << DBE.driverText();
	}
}

void ODBSQL::updateObject( Object &pObject )
{
	ObjectData	&D = data( pObject );

	QSqlQuery			 Q;

	Q.prepare( "UPDATE object SET parent = :parent, name = :name, aliases = :aliases, player = :player, connection = :connection, owner = :owner, location = :location, programmer = :programmer, wizard = :wizard, read = :read, write = :write, fertile = :fertile WHERE id = :id" );

	bindObject( D, Q );

	Q.exec();

	ObjectManager::instance()->recordWrite();

	QSqlError		DBE = Q.lastError();

	if( DBE.type() != QSqlError::NoError )
	{
		qDebug() << DBE.databaseText() << DBE.driverText();
	}

	D.mLastWrite = ObjectManager::timestamp();
}

void ODBSQL::addVerb( Object &pObject, QString pName )
{
	Verb		*V = pObject.verb( pName );

	if( !V )
	{
		return;
	}

	FuncData	&FD = funcdata( *V );
	VerbData	&VD = verbdata( *V );

	static QSqlQuery			 Q;
	static bool					 B = false;

	if( !B )
	{
		B = Q.prepare( "INSERT INTO verb "
					"( name, object, owner, read, write, execute, script, dobj, preptype, iobj, prep, aliases, code )"
					"VALUES "
					"( :name, :object, :owner, :read, :write, :execute, :script, :dobj, :preptype, :iobj, :prep, :aliases, :code )"
					);
	}

	bindFunc( FD, Q );
	bindVerb( VD, Q );

	Q.exec();

	ObjectManager::instance()->recordWrite();

	QSqlError		DBE = Q.lastError();

	if( DBE.type() != QSqlError::NoError )
	{
		qDebug() << DBE.databaseText() << DBE.driverText();
	}
}

void ODBSQL::deleteVerb( Object &pObject, QString pName )
{
	static QSqlQuery			 Q;
	static bool					 B = false;

	if( !B )
	{
		B = Q.prepare( "DELETE FROM verb WHERE object = :object AND name = :name" );
	}

	Q.bindValue( ":object", pObject.id() );
	Q.bindValue( ":name",   pName );

	Q.exec();

	ObjectManager::instance()->recordWrite();

	QSqlError		DBE = Q.lastError();

	if( DBE.type() != QSqlError::NoError )
	{
		qDebug() << DBE.databaseText() << DBE.driverText();
	}
}

void ODBSQL::updateVerb(Object &pObject, QString pName)
{
	Verb		*V = pObject.verb( pName );

	if( !V )
	{
		return;
	}

	FuncData	&FD = funcdata( *V );
	VerbData	&VD = verbdata( *V );

	static QSqlQuery			 Q;
	static bool					 B = false;

	if( !B )
	{
		B = Q.prepare( "UPDATE verb SET "
					"owner = :owner, read = :read, write = :write, execute = :execute, script = :script, dobj = :dobj, preptype = :preptype, iobj = :iobj, prep = :prep, aliases = :aliases, code = :code "
					"WHERE object = :object AND name = :name"
					);
	}

	bindFunc( FD, Q );
	bindVerb( VD, Q );

	Q.exec();

	ObjectManager::instance()->recordWrite();

	QSqlError		DBE = Q.lastError();

	if( DBE.type() != QSqlError::NoError )
	{
		qDebug() << DBE.databaseText() << DBE.driverText();
	}
}

void ODBSQL::addProperty(Object &pObject, QString pName)
{
	Property		*P = pObject.prop( pName );

	if( !P )
	{
		return;
	}

	PropertyData	&PD = data( *P );

	static QSqlQuery			 Q;
	static bool					 B = false;

	if( !B )
	{
		B = Q.prepare( "INSERT INTO property "
					   "( name, object, parent, owner, read, write, change, type, value )"
					   "VALUES "
					   "( :name, :object, :parent, :owner, :read, :write, :change, :type, :value )"
					   );

	}

	bindProperty( PD, Q );

	Q.exec();

	ObjectManager::instance()->recordWrite();

	QSqlError		DBE = Q.lastError();

	if( DBE.type() != QSqlError::NoError )
	{
		qDebug() << DBE.databaseText() << DBE.driverText();
	}
}

void ODBSQL::deleteProperty( Object &pObject, QString pName )
{
	static QSqlQuery			 Q;
	static bool					 B = false;

	if( !B )
	{
		B = Q.prepare( "DELETE FROM property WHERE object = :object AND name = :name" );
	}

	Q.bindValue( ":object", pObject.id() );
	Q.bindValue( ":name",   pName );

	Q.exec();

	ObjectManager::instance()->recordWrite();

	QSqlError		DBE = Q.lastError();

	if( DBE.type() != QSqlError::NoError )
	{
		qDebug() << DBE.databaseText() << DBE.driverText();
	}
}

void ODBSQL::updateProperty( Object &pObject, QString pName )
{
	Property		*P = pObject.prop( pName );

	if( !P )
	{
		return;
	}

	PropertyData	&PD = data( *P );

	static QSqlQuery			 Q;
	static bool					 B = false;

	if( !B )
	{
		B = Q.prepare( "UPDATE property SET "
					   "parent = :parent, owner = :owner, read = :read, write = :write, change = :change, type = :type, value = :value "
					   "WHERE name = :name AND object = :object"
					   );
	}

	bindProperty( PD, Q );

	Q.exec();

	ObjectManager::instance()->recordWrite();

	QSqlError		DBE = Q.lastError();

	if( DBE.type() != QSqlError::NoError )
	{
		qDebug() << DBE.databaseText() << DBE.driverText();
	}
}

ObjectId ODBSQL::findPlayer( QString pName ) const
{
	if( !mDB.isOpen() )
	{
		return( OBJECT_NONE );
	}

	QSqlQuery	Q1;

	Q1.prepare( "SELECT id FROM object WHERE lower( name ) = lower( :name ) AND player" );

	Q1.bindValue( ":name", pName );

	if( !Q1.exec() )
	{
		return( OBJECT_NONE );
	}

	ObjectManager::instance()->recordRead();

	if( !Q1.next() )
	{
		return( OBJECT_NONE );
	}

	return( Q1.value( 0 ).toInt() );
}

ObjectId ODBSQL::findByProp( QString pName, const QVariant &pValue ) const
{
	if( !mDB.isOpen() )
	{
		return( OBJECT_NONE );
	}

	QSqlQuery	Q1;

	Q1.prepare( "SELECT object FROM property WHERE name = :name AND value = :value" );

	Q1.bindValue( ":name", pName );
	Q1.bindValue( ":value", pValue );

	if( !Q1.exec() )
	{
		return( OBJECT_NONE );
	}

	ObjectManager::instance()->recordRead();

	if( !Q1.next() )
	{
		return( OBJECT_NONE );
	}

	return( Q1.value( 0 ).toInt() );
}

void ODBSQL::addTask( TaskEntry &TE )
{
	static QSqlQuery			 Q;
	static bool					 B = false;

	if( !B )
	{
		B = Q.prepare( "INSERT INTO task "
					"( id, timestamp, command, player, connection )"
					"VALUES "
					"( :id, :timestamp, :command, :player, :connection )"
					);
	}

	Q.bindValue( ":id", TE.id() );
	Q.bindValue( ":timestamp", TE.timestamp() );
	Q.bindValue( ":command", TE.command() );
	Q.bindValue( ":player", TE.playerid() );
	Q.bindValue( ":connection", TE.connectionid() );

	Q.exec();
}

QList<TaskEntry> ODBSQL::tasks( qint64 pTimeStamp )
{
	QList<TaskEntry>			TskLst;

	static QSqlQuery			 Q1;
	static bool					 B1 = false;

	if( !B1 )
	{
		B1 = Q1.prepare( "SELECT * FROM task WHERE timestamp <= :timestamp ORDER BY timestamp ASC" );
	}

	Q1.bindValue( ":timestamp", pTimeStamp );

	Q1.exec();

	while( Q1.next() )
	{
		TaskEntry		TE;
		TaskEntryData	&TD = data( TE );

		TD.mId        = Q1.value( "id" ).toInt();
		TD.mTimeStamp = Q1.value( "timestamp" ).toLongLong();
		TD.mCommand   = Q1.value( "command" ).toString();
		TD.mPlayerId  = Q1.value( "player" ).toInt();
		TD.mConnectionId = Q1.value( "connection" ).toInt();

		TD.TID = qMax( TD.TID, TD.mId + 1 );

		TskLst << TE;
	}

	static QSqlQuery			 Q2;
	static bool					 B2 = false;

	if( !B2 )
	{
		B2 = Q2.prepare( "DELETE FROM task WHERE timestamp <= :timestamp" );
	}

	Q2.bindValue( ":timestamp", pTimeStamp );

	Q2.exec();

	return( TskLst );
}

qint64 ODBSQL::nextTaskTime()
{
	static QSqlQuery			 Q;
	static bool					 B = false;

	if( !B )
	{
		B = Q.prepare( "SELECT timestamp FROM task ORDER BY timestamp ASC LIMIT 1" );
	}

	Q.exec();

	return( Q.next() ? Q.value( "timestamp" ).toLongLong() : -1 );
}

void ODBSQL::killTask( TaskId pTaskId )
{
	static QSqlQuery			 Q;
	static bool					 B = false;

	if( !B )
	{
		B = Q.prepare( "DELETE FROM task WHERE id = :id" );
	}

	Q.bindValue( ":id", pTaskId );

	Q.exec();
}

void ODBSQL::checkpoint()
{

}

