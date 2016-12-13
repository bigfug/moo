#ifndef ODBFILE_H
#define ODBFILE_H

#include <QString>

#include "odb.h"
#include "objectmanager.h"
#include "taskentry.h"

class Object;
class Verb;
class Property;

class ODBFile : public ODB
{
	Q_OBJECT

public:
	ODBFile( const QString &pFileName = "" );

	virtual ~ODBFile( void ) {}

	virtual void load( void ) Q_DECL_OVERRIDE;
	virtual void save( void ) Q_DECL_OVERRIDE;

private:
	void loadObject( QDataStream &DS, Object &O );
	void saveObject( QDataStream &DS, const Object &O );

	void loadVerb( QDataStream &DS, Verb &V );
	void saveVerb( QDataStream &DS, const Verb &V );

	void loadProperty( QDataStream &DS, Property &P );
	void saveProperty( QDataStream &DS, const Property &P );

	void loadTask( QDataStream &DS, TaskEntry &TE );
	void saveTask( QDataStream &DS, const TaskEntry &TE );

private:
	QString			 mFileName;
};

#endif // ODBFILE_H
