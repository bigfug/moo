#include "editorapplication.h"

#include <QDebug>

#include "cursesreader.h"

EditorApplication::EditorApplication( int &argc, char **argv )
	: QCoreApplication( argc, argv )
{
	mInputThread = new CursesReader();

	connect( mInputThread, &CursesReader::input, &mEditor, &Editor::input );

	connect( mInputThread, SIGNAL(resized(int,int)), &mEditor, SLOT(setSize(int,int)) );

	connect( mInputThread, &QThread::finished, mInputThread, &QObject::deleteLater );

	mInputThread->start();

	connect( &mEditor, &Editor::quit, this, &EditorApplication::quit );

	connect( &mEditor, &Editor::output, mInputThread, &CursesReader::output );

	QStringList		Text;

	for( int i = 0 ; i < 50 ; i++ )
	{
		Text << QString( "Line %1" ).arg( i );
	}

	Text << QString();

	mEditor.setText( Text );

	mEditor.redraw();
}

EditorApplication::~EditorApplication()
{
	mEditor.clearScreen();
}
