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

private:
	//void updateObject( const Object &O );

private:
	QSqlDatabase			 mDB;
};

#endif // ODBSQL_H
