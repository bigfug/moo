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

	Text << "Hello, World!";
	Text << "Line 2";

	mEditor.setText( Text );

	mEditor.redraw();
}
