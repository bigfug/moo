#ifndef TASK_ROLLBACK_H
#define TASK_ROLLBACK_H

#include <QObject>

#include <QtTest/QtTest>

#include "luatestdata.h"

class TaskRollback : public LuaTestObject
{
	Q_OBJECT

private slots:
	void taskRollbackObjectCreate( void );
	void taskRollbackObjectRecycle( void );

	void taskRollbackObjectRead( void );
	void taskRollbackObjectWrite( void );
	void taskRollbackObjectFertile( void );
	void taskRollbackObjectName( void );
	void taskRollbackObjectOwner( void );
	void taskRollbackObjectParent( void );
	void taskRollbackObjectLocation( void );
	void taskRollbackObjectPlayer( void );
	void taskRollbackObjectProgrammer( void );
	void taskRollbackObjectWizard( void );

	void taskRollbackObjectPropAdd( void );
	void taskRollbackObjectPropDelete( void );
	void taskRollbackObjectPropClear( void );
	void taskRollbackObjectPropValue( void );
	void taskRollbackObjectPropInherit( void );

	void taskRollbackObjectAliasAdd();
	void taskRollbackObjectAliasDelete();
};

#endif // TASK_ROLLBACK_H
