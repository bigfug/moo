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

	virtual ~ODBSQL( void ) {}

	virtual void load( void ) Q_DECL_OVERRIDE;
	virtual void save( void ) Q_DECL_OVERRIDE;

	virtual void saveObject( Object *O ) Q_DECL_OVERRIDE;

	virtual Object *object( ObjectId pIndex ) const Q_DECL_OVERRIDE;

	virtual void registerObject( const Object &pObject ) Q_DECL_OVERRIDE;

	virtual ObjectId findPlayer( QString pName ) const Q_DECL_OVERRIDE;

private:
//	void loadObject( QDataStream &DS, Object &O );
	void saveObject( const Object &O );

//	void loadVerb( QDataStream &DS, Verb &V );
//	void saveVerb( QDataStream &DS, const Verb &V );

//	void loadProperty( QDataStream &DS, Property &P );
//	void saveProperty( QDataStream &DS, const Property &P );

//	void loadTask( QDataStream &DS, TaskEntry &TE );
//	void saveTask( QDataStream &DS, const TaskEntry &TE );

private:
	QSqlDatabase			 mDB;
};

#endif // ODBSQL_H
