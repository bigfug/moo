#ifndef EVAL_MOO_H
#define EVAL_MOO_H

#include <QObject>

#include <QtTest/QtTest>

#include "luatestdata.h"

class eval_moo : public LuaTestObject
{
	Q_OBJECT

private slots:
	void broadcast_data( void );
	void broadcast( void );

	void notify( void );
	void root( void );
	void system( void );

	void eval_data( void );
	void eval( void );

	void evalArgs_data( void );
	void evalArgs( void );
};

#endif // EVAL_MOO_H
