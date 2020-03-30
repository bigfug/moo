#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>

#include "syntaxhighlighterlua.h"

class CodeEditor : public QPlainTextEdit
{
	Q_OBJECT

public:
	explicit CodeEditor(QWidget *parent = 0);

	void lineNumberAreaPaintEvent(QPaintEvent *event);

	int lineNumberAreaWidth();

	void updateHighlighter();

protected:
	void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

	virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

public slots:
	void highlightCurrentLine();

private slots:
	void updateLineNumberAreaWidth(int newBlockCount);
	void updateLineNumberArea(const QRect &, int);

private:
	QWidget								*lineNumberArea;
	SyntaxHighlighterLua		*mHighlighter;
};

#endif // CODEEDITOR_H
