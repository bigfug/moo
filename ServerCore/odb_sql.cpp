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

void ODBSQL::initialiseDatabase( QSqlDatabase &pDB )
{
	if( !pDB.tables().contains( "object" ) )
	{
		pDB.exec( "CREATE TABLE object ( "
				  "id INTEGER PRIMARY KEY,"
				  "name VARCHAR(255),"
				  "aliases TEXT,"
				  "parent INTEGER DEFAULT -1,"
				  "module INTEGER DEFAULT -1,"
				  "player BOOLEAN DEFAULT false,"
				  "connection INTEGER DEFAULT -1,"
				  "owner INTEGER DEFAULT -1,"
				  "location INTEGER DEFAULT -1,"
				  "programmer BOOLEAN DEFAULT false,"
				  "wizard BOOLEAN DEFAULT false,"
				  "read BOOLEAN DEFAULT true,"
				  "write BOOLEAN DEFAULT false,"
				  "fertile BOOLEAN DEFAULT false,"
				  "recycled BOOLEAN DEFAULT false"
				  ")" );

		QSqlError		DBE = pDB.lastError();

		if( DBE.type() != QSqlError::NoError )
		{
			qCritical() << "CREATE TABLE object:" << DBE.databaseText() << DBE.driverText();

			return;
		}
	}
	else
	{
		// check for existence of object.module

		updateObjectAddModule( pDB );
	}

	if( !pDB.tables().contains( "verb" ) )
	{
		pDB.exec( "CREATE TABLE verb ( "
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

		QSqlError		DBE = pDB.lastError();

		if( DBE.type() != QSqlError::NoError )
		{
			qCritical() << "CREATE TABLE verb:" << DBE.databaseText() << DBE.driverText();

			return;
		}
	}

	if( !pDB.tables().contains( "property" ) )
	{
		pDB.exec( "CREATE TABLE property ( "
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

		QSqlError		DBE = pDB.lastError();

		if( DBE.type() != QSqlError::NoError )
		{
			qCritical() << "CREATE TABLE property:" << DBE.databaseText() << DBE.driverText();

			return;
		}
	}

	if( !pDB.tables().contains( "task" ) )
	{
		pDB.exec( "CREATE TABLE task ( "
				  "id INTEGER,"
				  "timestamp INTEGER,"
				  "command TEXT,"
				  "player INTEGER DEFAULT -1,"
				  "connection INTEGER DEFAULT -1,"
				  "minute TEXT,"
				  "hour TEXT,"
				  "day_of_week TEXT,"
				  "day_of_month TEXT,"
				  "month TEXT,"
				  "year TEXT"
				  ")" );

		QSqlError		DBE = pDB.lastError();

		if( DBE.type() != QSqlError::NoError )
		{
			qCritical() << "CREATE TABLE task:" << DBE.databaseText() << DBE.driverText();

			return;
		}
	}
	else
	{
		// check for existence of object.module

		updateTaskAddSchedule( pDB );
	}

	if( !pDB.tables().contains( "signals" ) )
	{
		pDB.exec( "CREATE TABLE signals ( "
				  "src_object INTEGER DEFAULT -1,"
				  "src_verb TEXT,"
				  "dst_object INTEGER DEFAULT -1,"
				  "dst_verb TEXT,"
				  "FOREIGN KEY(`src_object`) REFERENCES `object`(`id`),"
				  "FOREIGN KEY(`dst_object`) REFERENCES `object`(`id`)"
				  ")" );

		QSqlError		DBE = pDB.lastError();

		if( DBE.type() != QSqlError::NoError )
		{
			qCritical() << "CREATE TABLE signals:" << DBE.databaseText() << DBE.driverText();

			return;
		}
	}

	if( true )
	{
		QSqlQuery	Q( "SELECT id FROM object WHERE recycled = true" );

		QSqlQuery	DeletePropertiesQuery;
		QSqlQuery	DeleteVerbsQuery;
		QSqlQuery	DeleteTasksQuery;

		DeletePropertiesQuery.prepare( "DELETE FROM property WHERE object = :id OR owner = :id" );

		DeleteVerbsQuery.prepare( "DELETE FROM verb WHERE object = :id OR owner = :id" );

		DeleteTasksQuery.prepare( "DELETE FROM task WHERE player = :id" );

		if( Q.exec() )
		{
			while( Q.next() )
			{
				int		id = Q.value( 0 ).toInt();

				DeletePropertiesQuery.bindValue( ":id", id );
				DeleteVerbsQuery.bindValue( ":id", id );
				DeleteTasksQuery.bindValue( ":id", id );

				DeletePropertiesQuery.exec();
				DeleteVerbsQuery.exec();
				DeleteTasksQuery.exec();
			}
		}

		pDB.exec( "DELETE FROM object WHERE recycled = true" );
	}
}

ODBSQL::ODBSQL()
{
	QSqlDatabase		DB = QSqlDatabase::addDatabase( "QSQLITE" );

	DB.setDatabaseName( "moo.db" );

	if( !DB.open() )
	{
		return;
	}

	initialiseDatabase( DB );

	// clear all verb code

	QSqlQuery		Q = DB.exec( "UPDATE verb SET code = ''" );
}

void ODBSQL::load()
{
	QSqlDatabase				 DB = QSqlDatabase::database();
	ObjectManager				&OM = *ObjectManager::instance();
	ObjectManagerData			&Data = data( OM );

	if( !DB.isOpen() )
	{
		return;
	}

	QSqlQuery	Q1 = DB.exec( "SELECT MAX( id ) FROM object" );

	if( Q1.next() )
	{
		Data.mObjNum = Q1.value( 0 ).toInt() + 1;
	}

	ObjectManager::instance()->recordRead();

	// Load all the player objects that were previously connected

	QSqlQuery	Q2 = DB.exec( "SELECT id FROM object WHERE player AND connection != -1 AND !recycled" );

	while( Q2.next() )
	{
		ObjectManager::instance()->recordRead();

		ObjectManager::instance()->object( Q2.value( 0 ).toInt() );
	}

	// Set any scheduled tasks to their next time

	qint64						CurrTime = QDateTime::currentMSecsSinceEpoch();

	QSqlQuery					 Q3;

	Q3.prepare( "SELECT * FROM task WHERE timestamp <= :timestamp ORDER BY timestamp ASC" );

	Q3.bindValue( ":timestamp", CurrTime );

	Q3.exec();

	while( Q3.next() )
	{
		TaskEntry		TE;

		queryToTaskData( Q3, data( TE ) );

		TE.updateTimestampFromSchedule( CurrTime );

		if( TE.timestamp() > CurrTime )
		{
			QSqlQuery			UpdateQuery;

			UpdateQuery.prepare( "UPDATE task SET timestamp = :timestamp WHERE id = :id" );

			UpdateQuery.bindValue( ":timestamp", TE.timestamp() );
			UpdateQuery.bindValue( ":id", TE.id() );

			UpdateQuery.exec();
		}
	}

	QSqlQuery			 Q4;

	Q4.prepare( "DELETE FROM task WHERE timestamp <= :timestamp" );

	Q4.bindValue( ":timestamp", CurrTime );

	Q4.exec();
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

	queryToObjectData( Q, D );

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

			queryToVerbData( Q, FD, VD );

			if( FD.mDirty && !V.compile() )
			{
				FD.mDirty = false;

				ObjectManager::instance()->updateVerb( O, FD.mName );
			}

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

			queryToPropertyData( Q, PD );

			D.mProperties.insert( P.name(), P );
		}
	}

	ObjectManager::instance()->recordRead();

	D.mSignalConnections = signalConnections( D.mId );

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
	Q.bindValue( ":module", D.mModule );
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
				{
					Q.bindValue( ":type", "json" );

					QVariantMap		PrpDat = D.mValue.toMap();

					lua_util::objectsToStrings( PrpDat );

					QJsonDocument	Json = QJsonDocument::fromVariant( PrpDat );

					Q.bindValue( ":value", Json.toJson( QJsonDocument::Compact ) );

					qDebug() << Json.toJson( QJsonDocument::Compact );
				}
				break;
		}
	}
}

void ODBSQL::insertObjectData( QSqlQuery &Q, const ObjectData &OD )
{
	Q.prepare( "INSERT INTO object "
			   "( id, parent, module, name, aliases, player, connection, owner, location, programmer, wizard, read, write, fertile, recycled ) "
			   "VALUES "
			   "( :id, :parent, :module, :name, :aliases, :player, :connection, :owner, :location, :programmer, :wizard, :read, :write, :fertile, :recycled )" );

	bindObject( OD, Q );

	Q.exec();
}

void ODBSQL::insertVerbData( QSqlQuery &Q, const FuncData &FD, const VerbData &VD )
{
	Q.prepare( "INSERT INTO verb "
				"( name, object, owner, read, write, execute, script, dobj, preptype, iobj, prep, aliases, code )"
				"VALUES "
				"( :name, :object, :owner, :read, :write, :execute, :script, :dobj, :preptype, :iobj, :prep, :aliases, :code )"
				);

	bindFunc( FD, Q );
	bindVerb( VD, Q );

	Q.exec();
}

void ODBSQL::insertPropertyData( QSqlQuery &Q, const PropertyData &PD )
{
	Q.prepare( "INSERT INTO property "
			   "( name, object, parent, owner, read, write, change, type, value )"
			   "VALUES "
			   "( :name, :object, :parent, :owner, :read, :write, :change, :type, :value )"
			   );

	bindProperty( PD, Q );

	Q.exec();
}

void ODBSQL::insertTaskData( QSqlQuery &Q, const TaskEntryData &D )
{
	Q.prepare( "INSERT INTO task "
			   "( id, timestamp, command, player, connection, minute, hour, day_of_week, day_of_month, month, year )"
			   "VALUES "
			   "( :id, :timestamp, :command, :player, :connection, :minute, :hour, :day_of_week, :day_of_month, :month, :year )"
			   );

	Q.bindValue( ":id", D.mId );
	Q.bindValue( ":timestamp", D.mTimeStamp );
	Q.bindValue( ":command", D.mCommand );
	Q.bindValue( ":player", D.mPlayerId );
	Q.bindValue( ":connection", D.mConnectionId );

	const TaskEntrySchedule	&S = D.mSchedule;

	Q.bindValue( ":minute", S.mMinute );
	Q.bindValue( ":hour", S.mHour );
	Q.bindValue( ":day_of_week", S.mDayOfWeek );
	Q.bindValue( ":day_of_month", S.mDayOfMonth );
	Q.bindValue( ":month", S.mMonth );
	Q.bindValue( ":year", S.mYear );

	Q.exec();
}

void ODBSQL::insertSignalData( QSqlQuery &Q, const SignalConnection &SC )
{
	Q.prepare( "INSERT INTO signals "
			   "( src_object, src_verb, dst_object, dst_verb )"
			   "VALUES "
			   "( :src_object, :src_verb, :dst_object, :dst_verb )"
			   );

	Q.bindValue( ":src_object", SC.mSrcObj );
	Q.bindValue( ":src_verb", SC.mSrcVrb );
	Q.bindValue( ":dst_object", SC.mDstObj );
	Q.bindValue( ":dst_verb", SC.mDstVrb );

	Q.exec();
}

void ODBSQL::addObject( Object &pObject )
{
	ObjectData	&D = data( pObject );

	QSqlQuery			 Q;

	insertObjectData( Q, D );

	ObjectManager::instance()->recordWrite();

	QSqlError		DBE = Q.lastError();

	if( DBE.type() != QSqlError::NoError )
	{
		qCritical() << "addObject:" << DBE.databaseText() << DBE.driverText();
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
		qCritical() << "deleteObject:" << DBE.databaseText() << DBE.driverText();
	}
}

void ODBSQL::updateObject( Object &pObject )
{
	ObjectData	&D = data( pObject );

	QSqlQuery			 Q;

	Q.prepare( "UPDATE object SET "
			   "parent = :parent, module = :module, name = :name, aliases = :aliases, player = :player, connection = :connection, "
			   "owner = :owner, location = :location, programmer = :programmer, wizard = :wizard, read = :read, "
			   "write = :write, fertile = :fertile, recycled = :recycled "
			   "WHERE id = :id" );

	bindObject( D, Q );

	Q.exec();

	ObjectManager::instance()->recordWrite();

	QSqlError		DBE = Q.lastError();

	if( DBE.type() != QSqlError::NoError )
	{
		qCritical() << "updateObject:" << DBE.databaseText() << DBE.driverText();
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

	QSqlQuery			 Q;

	insertVerbData( Q, FD, VD );

	ObjectManager::instance()->recordWrite();

	QSqlError		DBE = Q.lastError();

	if( DBE.type() != QSqlError::NoError )
	{
		qCritical() << "addVerb:" << DBE.databaseText() << DBE.driverText();
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
		qCritical() << "deleteVerb:" << DBE.databaseText() << DBE.driverText();
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
					   "owner = :owner, read = :read, write = :write, execute = :execute, script = :script, dobj = :dobj, "
					   "preptype = :preptype, iobj = :iobj, prep = :prep, aliases = :aliases, code = :code "
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
		qCritical() << "updateVerb:" << DBE.databaseText() << DBE.driverText();
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

	QSqlQuery		 Q;

	insertPropertyData( Q, PD );

	ObjectManager::instance()->recordWrite();

	QSqlError		DBE = Q.lastError();

	if( DBE.type() != QSqlError::NoError )
	{
		qCritical() << "addProperty:" << DBE.databaseText() << DBE.driverText();
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
		qCritical() << "deleteProperty:" << DBE.databaseText() << DBE.driverText();
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
		qCritical() << "updateProperty:" << DBE.databaseText() << DBE.driverText();
	}
}

ObjectId ODBSQL::findPlayer( QString pName ) const
{
	if( !QSqlDatabase::database().isOpen() )
	{
		return( OBJECT_NONE );
	}

	QSqlQuery	Q1;

	Q1.prepare( "SELECT id FROM object WHERE lower( name ) = lower( :name ) AND player AND recycled = false" );

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
	if( !QSqlDatabase::database().isOpen() )
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
	QSqlQuery			 Q;

	insertTaskData( Q, data( TE ) );
}

void ODBSQL::addSignalConnection( const SignalConnection &SC )
{
	QSqlQuery			 Q;

//	Q.exec( "SELECT COUNT( * ) FROM signals WHERE src_object = :src_object AND src_verb = :src_verb AND dst_object = :dst_object AND dst_verb = :dst_verb" );

//	Q.bindValue( ":src_object", SC.mSrcObj );
//	Q.bindValue( ":src_verb", SC.mSrcVrb );
//	Q.bindValue( ":dst_object", SC.mDstObj );
//	Q.bindValue( ":dst_verb", SC.mDstVrb );

//	if( Q.exec() && Q.next() && Q.value( 0 ).toInt() == 0 )
	{
		insertSignalData( Q, SC );
	}
}

void ODBSQL::deleteSignalConnection( const SignalConnection &SC )
{
	QSqlQuery			 Q;

	if( SC.mDstVrb.isEmpty() )
	{
		if( SC.mDstObj == OBJECT_NONE )
		{
			if( SC.mSrcVrb.isEmpty() )
			{
				Q.prepare( "DELETE FROM signals WHERE src_object = :src_object" );
			}
			else
			{
				Q.prepare( "DELETE FROM signals WHERE src_object = :src_object AND src_verb = :src_verb" );
			}
		}
		else
		{
			Q.prepare( "DELETE FROM signals WHERE src_object = :src_object AND src_verb = :src_verb AND dst_object = :dst_object" );
		}
	}
	else
	{
		Q.prepare( "DELETE FROM signals WHERE src_object = :src_object AND src_verb = :src_verb AND dst_object = :dst_object AND dst_verb = :dst_verb" );
	}

	Q.bindValue( ":src_object", SC.mSrcObj );
	Q.bindValue( ":src_verb", SC.mSrcVrb );
	Q.bindValue( ":dst_object", SC.mDstObj );
	Q.bindValue( ":dst_verb", SC.mDstVrb );

	Q.exec();
}

QVector<SignalConnection> ODBSQL::signalConnections( ObjectId pSrcObj ) const
{
	QSqlQuery			 Q;

	Q.prepare( "SELECT * FROM signals WHERE src_object = :src_object" );

	Q.bindValue( ":src_object", pSrcObj );

	QVector<SignalConnection>	SigLst;

	Q.exec();

	while( Q.next() )
	{
		SignalConnection		SC;

		queryToSignalData( Q, SC );

		SigLst << SC;
	}

	return( SigLst );
}

QList<TaskEntry> ODBSQL::tasks( qint64 pTimeStamp )
{
	QSqlDatabase				DB = QSqlDatabase::database();

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

		queryToTaskData( Q1, data( TE ) );

		TskLst << TE;

		TE.updateTimestampFromSchedule( pTimeStamp );

		if( TE.timestamp() > pTimeStamp )
		{
			QSqlQuery			UpdateQuery;

			UpdateQuery.prepare( "UPDATE task SET timestamp = :timestamp WHERE id = :id" );

			UpdateQuery.bindValue( ":timestamp", TE.timestamp() );
			UpdateQuery.bindValue( ":id", TE.id() );

			UpdateQuery.exec();
		}
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

ObjectIdVector ODBSQL::children( ObjectId pParentId ) const
{
	ObjectIdVector		ChildVector;

	if( !QSqlDatabase::database().isOpen() )
	{
		return( ChildVector );
	}

	QSqlQuery	Q;

	Q.prepare( "SELECT id FROM object WHERE parent = :parent AND recycled = false" );

	Q.bindValue( ":parent", pParentId );

	if( !Q.exec() )
	{
		return( ChildVector );
	}

	while( Q.next() )
	{
		ChildVector << ObjectId( Q.value( 0 ).toInt() );
	}

	return( ChildVector );
}

int ODBSQL::childrenCount( ObjectId pParentId ) const
{
	if( !QSqlDatabase::database().isOpen() )
	{
		return( 0 );
	}

	QSqlQuery	Q;

	Q.prepare( "SELECT COUNT( id ) FROM object WHERE parent = :parent AND recycled = false" );

	Q.bindValue( ":parent", pParentId );

	if( !Q.exec() || !Q.next() )
	{
		return( 0 );
	}

	return( Q.value( 0 ).toInt() );
}

QMap<ObjectId,QString> ODBSQL::objectNames( ObjectIdVector pIds ) const
{
	QMap<ObjectId,QString>		ObjectNames;

	if( !QSqlDatabase::database().isOpen() )
	{
		return( ObjectNames );
	}

	QStringList		OIDLst;

	for( ObjectId OID : pIds )
	{
		OIDLst << QString::number( OID );
	}

	QSqlQuery	Q;

	Q.prepare( "SELECT id, name FROM object WHERE id IN ( :id ) AND recycled = false" );

	Q.bindValue( ":id", OIDLst );

	if( !Q.exec() )
	{
		return( ObjectNames );
	}

	while( Q.next() )
	{
		ObjectNames.insert( Q.value( 0 ).toInt(), Q.value( 1 ).toString() );
	}

	return( ObjectNames );
}

QString ODBSQL::objectName( ObjectId pId ) const
{
	if( !QSqlDatabase::database().isOpen() )
	{
		return( 0 );
	}

	QSqlQuery	Q;

	Q.prepare( "SELECT name FROM object WHERE id = :id AND recycled = false" );

	Q.bindValue( ":id", pId );

	if( !Q.exec() || !Q.next() )
	{
		return( QString() );
	}

	return( Q.value( 0 ).toString() );
}

ObjectId ODBSQL::objectParent(ObjectId pId) const
{
	if( !QSqlDatabase::database().isOpen() )
	{
		return( 0 );
	}

	QSqlQuery	Q;

	Q.prepare( "SELECT parent FROM object WHERE id = :id AND recycled = false" );

	Q.bindValue( ":id", pId );

	if( !Q.exec() || !Q.next() )
	{
		return( OBJECT_NONE );
	}

	return( Q.value( 0 ).toInt() );
}

void ODBSQL::updateObjectAddModule( QSqlDatabase &pDB )
{
	if( !pDB.isOpen() )
	{
		return;
	}

	if( findColumn( pDB, "object", "module" ) )
	{
		return;
	}

	QSqlQuery	Q( pDB );

	Q.prepare( "ALTER TABLE object ADD COLUMN module INTEGER DEFAULT -1" );

	if( !Q.exec() )
	{
		throw std::runtime_error( "Can't add column module to object" );
	}

	Q_ASSERT( findColumn( pDB, "object", "module" ) );
}

void ODBSQL::updateTaskAddSchedule( QSqlDatabase &pDB )
{
	if( !pDB.isOpen() )
	{
		return;
	}

	if( !findColumn( pDB, "task", "minute" ) )
	{
		pDB.exec( "ALTER TABLE task ADD COLUMN minute TEXT" );
	}

	if( !findColumn( pDB, "task", "hour" ) )
	{
		pDB.exec( "ALTER TABLE task ADD COLUMN hour TEXT" );
	}

	if( !findColumn( pDB, "task", "day_of_week" ) )
	{
		pDB.exec( "ALTER TABLE task ADD COLUMN day_of_week TEXT" );
	}

	if( !findColumn( pDB, "task", "day_of_month" ) )
	{
		pDB.exec( "ALTER TABLE task ADD COLUMN day_of_month TEXT" );
	}

	if( !findColumn( pDB, "task", "month" ) )
	{
		pDB.exec( "ALTER TABLE task ADD COLUMN month TEXT" );
	}

	if( !findColumn( pDB, "task", "year" ) )
	{
		pDB.exec( "ALTER TABLE task ADD COLUMN year TEXT" );
	}

	Q_ASSERT( findColumn( pDB, "task", "minute" ) );
	Q_ASSERT( findColumn( pDB, "task", "hour" ) );
	Q_ASSERT( findColumn( pDB, "task", "day_of_week" ) );
	Q_ASSERT( findColumn( pDB, "task", "day_of_month" ) );
	Q_ASSERT( findColumn( pDB, "task", "month" ) );
	Q_ASSERT( findColumn( pDB, "task", "year" ) );
}

bool ODBSQL::findColumn( const QString &pTable, const QString &pColumn ) const
{
	return( ODBSQL::findColumn( QSqlDatabase::database(), pTable, pColumn ) );
}

bool ODBSQL::findColumn( const QSqlDatabase &pDB, const QString &pTable, const QString &pColumn )
{
	QSqlQuery	Q( pDB );

	Q.exec( QString( "PRAGMA table_info( %1 );" ).arg( pTable ) );

	while( Q.next() )
	{
		if( Q.value( "name" ).toString() == pColumn )
		{
			return( true );
		}
	}

	return( false );
}

void ODBSQL::queryToObjectData( const QSqlQuery &Q, ObjectData &D )
{
	D.mId = Q.value( "id" ).toInt();
	D.mLocation = Q.value( "location" ).toInt();
	D.mName = Q.value( "name" ).toString();
	D.mAliases = Q.value( "aliases" ).toString().split( ',', QString::SkipEmptyParts );
	D.mFertile = Q.value( "fertile" ).toBool();
	D.mOwner = Q.value( "owner" ).toInt();
	D.mParent = Q.value( "parent" ).toInt();
	D.mModule = Q.value( "module" ).toInt();
	D.mPlayer = Q.value( "player" ).toBool();
	D.mConnection = Q.value( "connection" ).toInt();
	D.mProgrammer = Q.value( "programmer" ).toBool();
	D.mRead = Q.value( "read" ).toBool();
	D.mWizard = Q.value( "wizard" ).toBool();
	D.mWrite = Q.value( "write" ).toBool();
	D.mRecycled = Q.value( "recycled" ).toBool();
}

void ODBSQL::queryToVerbData( const QSqlQuery &Q, FuncData &FD, VerbData &VD )
{
	FD.mObject = Q.value( "object" ).toInt();
	FD.mName   = Q.value( "name" ).toString();

	FD.mExecute = Q.value( "execute" ).toBool();
	FD.mObject  = Q.value( "object" ).toInt();
	FD.mOwner   = Q.value( "owner" ).toInt();
	FD.mRead    = Q.value( "read" ).toBool();
	FD.mScript  = Q.value( "script" ).toString();
	FD.mWrite   = Q.value( "write" ).toBool();

	FD.mCompiled = Q.value( "code" ).toByteArray();
	FD.mDirty    = FD.mCompiled.isEmpty();

	VD.mAliases = Q.value( "aliases" ).toString().split( ',', QString::SkipEmptyParts );
	VD.mDirectObject = Verb::argobj_from( Q.value( "dobj" ).toString().toLatin1() );
	VD.mIndirectObject = Verb::argobj_from( Q.value( "iobj" ).toString().toLatin1() );
	VD.mPrepositionType = Verb::argobj_from( Q.value( "preptype" ).toString().toLatin1() );
	VD.mPreposition = Q.value( "prep" ).toString();
}

void ODBSQL::queryToPropertyData( const QSqlQuery &Q, PropertyData &PD )
{
	PD.mObject = Q.value( "object" ).toInt();
	PD.mName   = Q.value( "name" ).toString();
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

		if( PD.mValue.type() == QVariant::Map )
		{
			QVariantMap		VM = PD.mValue.toMap();

			lua_util::stringsToObjects( VM );

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
}

void ODBSQL::queryToTaskData( const QSqlQuery &Q, TaskEntryData &D )
{
	D.mId        = Q.value( "id" ).toInt();
	D.mTimeStamp = Q.value( "timestamp" ).toLongLong();
	D.mCommand   = Q.value( "command" ).toString();
	D.mPlayerId  = Q.value( "player" ).toInt();
	D.mConnectionId = Q.value( "connection" ).toInt();

	D.mSchedule.mMinute = Q.value( "minute" ).toString();
	D.mSchedule.mHour = Q.value( "hour" ).toString();
	D.mSchedule.mDayOfWeek = Q.value( "day_of_week" ).toString();
	D.mSchedule.mDayOfMonth = Q.value( "day_of_month" ).toString();
	D.mSchedule.mMonth = Q.value( "month" ).toString();
	D.mSchedule.mYear = Q.value( "year" ).toString();

	TaskEntry::setMaxTaskId( D.mId + 1 );
}

void ODBSQL::queryToSignalData( const QSqlQuery &Q, SignalConnection &SC )
{
	SC.mSrcObj = Q.value( "src_object" ).toInt();
	SC.mSrcVrb = Q.value( "src_verb" ).toString();
	SC.mDstObj = Q.value( "dst_object" ).toInt();
	SC.mDstVrb = Q.value( "dst_verb" ).toString();
}

void ODBSQL::exportModule( ObjectId pModuleId, const QString &pFileName, TransferInformation &pTrnInf ) const
{
	QSqlDatabase		DB1 = QSqlDatabase::database();
	QSqlDatabase		DB2 = QSqlDatabase::addDatabase( "QSQLITE", "EXPORT" );

	DB2.setDatabaseName( pFileName );

	qint64	s = QDateTime::currentMSecsSinceEpoch();

	if( !DB2.open() )
	{
		return;
	}

	try
	{
		initialiseDatabase( DB2 );

		QSqlQuery			Q1( DB1 );
		QSqlQuery			V1( DB1 );
		QSqlQuery			P1( DB1 );

		Q1.prepare( "SELECT * FROM object WHERE module = :module AND recycled = false" );
		V1.prepare( "SELECT * FROM verb WHERE object = :object" );
		P1.prepare( "SELECT * FROM property WHERE object = :object" );

		Q1.bindValue( ":module", pModuleId );

		if( !Q1.exec() )
		{
			return;
		}

		QSqlQuery		Q2( DB2 );

		ObjectData		OD;

		while( Q1.next() )
		{
			queryToObjectData( Q1, OD );

			insertObjectData( Q2, OD );

			pTrnInf.mObjects++;

			V1.bindValue( ":object", OD.mId );

			if( V1.exec() )
			{
				while( V1.next() )
				{
					FuncData		FD;
					VerbData		VD;

					queryToVerbData( V1, FD, VD );

					insertVerbData( Q2, FD, VD );

					pTrnInf.mVerbs++;
				}
			}

			P1.bindValue( ":object", OD.mId );

			if( P1.exec() )
			{
				while( P1.next() )
				{
					PropertyData	PD;

					queryToPropertyData( P1, PD );

					insertPropertyData( Q2, PD );

					pTrnInf.mProperties++;
				}
			}
		}
	}
	catch( const std::exception &e )
	{
		qWarning() << "export:" << QString::fromLatin1( e.what() );
	}

	DB2.close();

	pTrnInf.mMilliseconds = QDateTime::currentMSecsSinceEpoch() - s;
}

ObjectId ODBSQL::importModule( ObjectId pParentId, ObjectId pOwnerId, const QString &pFileName, TransferInformation &pTrnInf )
{
	QSqlDatabase		DB1 = QSqlDatabase::database();
	QSqlDatabase		DB2 = QSqlDatabase::addDatabase( "QSQLITE", "IMPORT" );

	DB2.setDatabaseName( pFileName );

	qint64	s = QDateTime::currentMSecsSinceEpoch();

	if( !DB2.open() )
	{
		return( OBJECT_NONE );
	}

	ObjectId			NewModuleId = OBJECT_NONE;

	try
	{
		NewModuleId = ObjectManager::instance()->newObjectId();

		//-----------------------------------------------------------------------
		// get the original module id

		QSqlQuery			O2( DB2 );

		if( !O2.exec(  "SELECT * FROM object WHERE id = module"  ) || !O2.next() )
		{
			throw std::runtime_error( "Can't get original module id" );
		}

		ObjectData			OD;

		queryToObjectData( O2, OD );

		ObjectId			OldModuleId = OD.mId;

		OD.mId       = NewModuleId;
		OD.mParent   = pParentId;
		OD.mModule   = NewModuleId;
		OD.mLocation = OBJECT_NONE;
		OD.mOwner    = pOwnerId;

		QMap<ObjectId,ObjectId>		ObjectIdMap;

		ObjectIdMap.insert( OldModuleId, NewModuleId );

		//-----------------------------------------------------------------------
		//

		if( !DB1.transaction() )
		{
			throw std::exception();
		}

		QSqlQuery			O1( DB1 );

		insertObjectData( O1, OD );

		pTrnInf.mObjects++;

		//-----------------------------------------------------------------------
		//

		if( !O2.exec( "SELECT * from object WHERE id != module" ) )
		{
			throw std::exception();
		}

		while( O2.next() )
		{
			ObjectId			NID = ObjectManager::instance()->newObjectId();

			queryToObjectData( O2, OD );

			ObjectIdMap.insert( OD.mId, NID );

			OD.mId       = NID;
			OD.mModule   = NewModuleId;

			insertObjectData( O1, OD );
		}

		//-----------------------------------------------------------------------
		// update parent, owner, and location on imported objects

		if( !O2.exec( "SELECT * from object" ) )
		{
			throw std::exception();
		}

		QSqlQuery		UO;

		UO.prepare( "UPDATE object SET parent = :parent, owner = :owner, location = :location WHERE id = :id" );

		while( O2.next() )
		{
			queryToObjectData( O2, OD );

			OD.mId       = ObjectIdMap.value( OD.mId, OBJECT_NONE );
			OD.mParent   = ObjectIdMap.value( OD.mParent, OBJECT_NONE );
			OD.mOwner    = ObjectIdMap.value( OD.mOwner, pOwnerId );
			OD.mLocation = ObjectIdMap.value( OD.mLocation, OBJECT_NONE );

			UO.bindValue( ":id",       OD.mId );
			UO.bindValue( ":parent",   OD.mParent );
			UO.bindValue( ":owner",    OD.mOwner );
			UO.bindValue( ":location", OD.mLocation );

			if( !UO.exec() )
			{
				throw std::exception();
			}
		}

		//-----------------------------------------------------------------------
		// insert verbs

		QSqlQuery			V1( DB1 );
		QSqlQuery			V2( DB2 );

		if( !V2.exec( "SELECT * FROM verb" ) )
		{
			throw std::exception();
		}

		while( V2.next() )
		{
			FuncData		FD;
			VerbData		VD;

			queryToVerbData( V2, FD, VD );

			FD.mOwner  = ObjectIdMap.value( FD.mOwner, pOwnerId );
			FD.mObject = ObjectIdMap.value( FD.mObject, OBJECT_NONE );

			if( FD.mObject != OBJECT_NONE )
			{
				insertVerbData( V1, FD, VD );

				pTrnInf.mVerbs++;
			}
		}

		//-----------------------------------------------------------------------
		// insert properties

		QSqlQuery			P1( DB1 );
		QSqlQuery			P2( DB2 );

		if( !P2.exec( "SELECT * FROM property" ) )
		{
			throw std::exception();
		}

		while( P2.next() )
		{
			PropertyData	PD;

			queryToPropertyData( P2, PD );

			PD.mOwner  = ObjectIdMap.value( PD.mOwner, pOwnerId );
			PD.mObject = ObjectIdMap.value( PD.mObject, OBJECT_NONE );
			PD.mParent = ObjectIdMap.value( PD.mParent, OBJECT_NONE );

			// lookup object ids on properties

			if( !strcmp( PD.mValue.typeName(), lua_object::luaHandle::mTypeName ) )
			{
				lua_object::luaHandle		H = PD.mValue.value<lua_object::luaHandle>();

				H.O = ObjectIdMap.value( H.O, OBJECT_NONE );

				PD.mValue = QVariant::fromValue( H );
			}

			// todo: need to process object id's on QVariantMap

			if( PD.mObject != OBJECT_NONE )
			{
				insertPropertyData( P1, PD );

				pTrnInf.mProperties++;
			}
		}

		DB1.commit();
	}
	catch( const std::exception &e )
	{
		qWarning() << "import:" << QString::fromLatin1( e.what() );

		DB1.rollback();
	}
	catch( ... )
	{
		DB1.rollback();
	}

	DB2.close();

	pTrnInf.mMilliseconds = QDateTime::currentMSecsSinceEpoch() - s;

	return( NewModuleId );
}

ObjectIdVector ODBSQL::connectedObjects() const
{
	ObjectIdVector		ObjectVector;

	if( !QSqlDatabase::database().isOpen() )
	{
		return( ObjectVector );
	}

	QSqlQuery	Q( "SELECT id FROM object WHERE connection != -1 AND recycled = false" );

	if( !Q.exec() )
	{
		return( ObjectVector );
	}

	while( Q.next() )
	{
		ObjectVector << ObjectId( Q.value( 0 ).toInt() );
	}

	return( ObjectVector );
}

