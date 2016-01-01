#ifndef PERMISSIONSTEST_H
#define PERMISSIONSTEST_H

#include <QObject>
#include <QtTest/QtTest>
#include "mooglobal.h"

class Object;
class Connection;
class ObjectManager;

class PermissionsTest : public QObject
{
	Q_OBJECT
public:
	explicit PermissionsTest( void );
	
signals:
	
private slots:
	void propAdd_data( void );
	void propAdd( void );
};

#endif // PERMISSIONSTEST_H
