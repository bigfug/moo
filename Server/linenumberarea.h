#ifndef LINENUMBERAREA_H
#define LINENUMBERAREA_H

#include <QWidget>

#include "codeeditor.h"

class LineNumberArea : public QWidget
{
	Q_OBJECT

public:
	LineNumberArea( CodeEditor *editor );

	QSize sizeHint() const {
		return QSize(codeEditor->lineNumberAreaWidth(), 0);
	}

protected:
	void paintEvent(QPaintEvent *event) {
		codeEditor->lineNumberAreaPaintEvent(event);
	}

private:
	CodeEditor *codeEditor;
};

#endif // LINENUMBERAREA_H
