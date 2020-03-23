#include <QCoreApplication>

#include "mooapplication.h"

int main( int argc, char *argv[] )
{
	QCoreApplication	a( argc, argv );

	MooApplication		m( a );

	m.process();

	int r = -1;

	if( m.initialiseApp() )
	{
		r = a.exec();

		m.deinitialiseApp();
	}

	return( r );
}
