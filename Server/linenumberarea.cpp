#include "linenumberarea.h"

LineNumberArea::LineNumberArea(CodeEditor *editor) : QWidget(editor)
{
	codeEditor = editor;

	QPalette	P = palette();

	P.setColor( QPalette::Base, QColor( "darkgray" ) );

	setPalette( P );
}
