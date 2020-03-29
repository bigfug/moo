#ifndef ODBSQL_H
#define ODBSQL_H

#include <QString>
#include <QSqlDatabase>

#include "odb.h"
#include "objectmanager.h"
#include "taskentry.h"

class Object;
class Verb;
class Property;

class ODBSQL : public ODB
{
	Q_OBJECT

public:
	ODBSQL( void );

	virtual ~ODBSQL( void ) Q_DECL_OVERRIDE {}

	virtual void load( void ) Q_DECL_OVERRIDE;
	virtual void save( void ) Q_DECL_OVERRIDE;

	virtual Object *object( ObjectId pIndex ) const Q_DECL_OVERRIDE;

	virtual bool hasObject( ObjectId pIndex ) const Q_DECL_OVERRIDE;

	virtual void addObject( Object &pObject ) Q_DECL_OVERRIDE;
	virtual void deleteObject( Object &pObject ) Q_DECL_OVERRIDE;
	virtual void updateObject( Object &pObject ) Q_DECL_OVERRIDE;

	virtual void addVerb( Object &pObject, QString pName ) Q_DECL_OVERRIDE;

	virtual void deleteVerb( Object &pObject, QString pName ) Q_DECL_OVERRIDE;

	virtual void updateVerb( Object &pObject, QString pName ) Q_DECL_OVERRIDE;

	virtual void addProperty( Object &pObject, QString pName ) Q_DECL_OVERRIDE;

	virtual void deleteProperty( Object &pObject, QString pName ) Q_DECL_OVERRIDE;

	virtual void updateProperty( Object &pObject, QString pName ) Q_DECL_OVERRIDE;

	virtual ObjectId findPlayer( QString pName ) const Q_DECL_OVERRIDE;
	virtual ObjectId findByProp( QString pName, const QVariant &pValue ) const Q_DECL_OVERRIDE;

	virtual void addTask( TaskEntry &TE ) Q_DECL_OVERRIDE;

	virtual QList<TaskEntry> tasks( qint64 pTimeStamp ) Q_DECL_OVERRIDE;

	virtual qint64 nextTaskTime( void ) Q_DECL_OVERRIDE;

	virtual void killTask( TaskId pTaskId ) Q_DECL_OVERRIDE;

	virtual void checkpoint( void ) Q_DECL_OVERRIDE;

	virtual ObjectIdVector children( ObjectId pParentId ) const Q_DECL_OVERRIDE;

	virtual int childrenCount( ObjectId pParentId ) const Q_DECL_OVERRIDE;

	virtual QMap<ObjectId,QString> objectNames( ObjectIdVector pIds ) const Q_DECL_OVERRIDE;

	virtual QString objectName( ObjectId pId ) const Q_DECL_OVERRIDE;

	virtual ObjectId objectParent( ObjectId pId ) const Q_DECL_OVERRIDE;

	virtual void exportModule( ObjectId pModuleId, const QString &pFileName ) const Q_DECL_OVERRIDE;

	virtual ObjectId importModule( ObjectId pParentId, ObjectId pOwnerId, const QString &pFileName ) Q_DECL_OVERRIDE;

private:
	//void updateObject( const Object &O );

	static void queryToObjectData( const QSqlQuery &Q, ObjectData &D );
	static void queryToVerbData( const QSqlQuery &Q, FuncData &FD, VerbData &VD );
	static void queryToPropertyData( const QSqlQuery &Q, PropertyData &PD );

	static void insertObjectData( QSqlQuery &Q, const ObjectData &OD );
	static void insertVerbData( QSqlQuery &Q, const FuncData &FD, const VerbData &VD );
	static void insertPropertyData( QSqlQuery &Q, const PropertyData &PD );

	static void initialiseDatabase( QSqlDatabase &pDB );

	static void updateObjectAddModule( QSqlDatabase &pDB );

	bool findColumn( const QString &pTable, const QString &pColumn ) const;

	static bool findColumn( const QSqlDatabase &pDB, const QString &pTable, const QString &pColumn );
};

#endif // ODBSQL_H
