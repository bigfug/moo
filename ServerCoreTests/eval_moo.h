#ifndef EVAL_MOO_H
#define EVAL_MOO_H

#include <QObject>

#include <QtTest/QtTest>

#include "luatestdata.h"

class eval_moo : public LuaTestObject
{
	Q_OBJECT

private slots:
	void broadcast( void );
	void broadcastElevated( void );

	void notify( void );
	void root( void );
	void system( void );

	void evalNotProgrammer( void );
	void evalProgrammer( void );
	void evalElevated( void );
};

#endif // EVAL_MOO_H
