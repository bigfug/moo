#include <QCoreApplication>

#include "editor.h"
#include "editorapplication.h"

int main( int argc, char *argv[] )
{
	EditorApplication a( argc, argv );

	return a.exec();
}
