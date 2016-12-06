#ifndef EDITORAPPLICATION_H
#define EDITORAPPLICATION_H

#include <QCoreApplication>
#include <QThread>

#include <editor.h>

class CursesReader;

class EditorApplication : public QCoreApplication
{
public:
	EditorApplication( int &argc, char **argv );

private:
	Editor				 mEditor;
	CursesReader		*mInputThread;
};

#endif // EDITORAPPLICATION_H
