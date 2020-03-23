
#include <QApplication>

#include "mainwindow.h"
#include "mooapplication.h"
#include "mooitemmodel.h"

int main( int argc, char *argv[] )
{
	QApplication	a( argc, argv );

	MooApplication	m( a );

	MainWindow		w;

	QObject::connect( &m, &MooApplication::message, &w, &MainWindow::log );

	m.process();

	w.show();

	int r = -1;

	if( m.initialiseApp() )
	{
		MooItemModel	*MooMod = new MooItemModel();

		w.installModel( MooMod );

		r = a.exec();

		m.deinitialiseApp();
	}

	return( r );
}
