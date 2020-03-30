#ifndef SYNTAXHIGHLIGHTERLUA_H
#define SYNTAXHIGHLIGHTERLUA_H

#include <QSyntaxHighlighter>

class SyntaxHighlighterLua : public QSyntaxHighlighter
{
	Q_OBJECT

public:
	explicit SyntaxHighlighterLua( QTextDocument *pParent = nullptr );

	virtual ~SyntaxHighlighterLua( void ) {}

	// SyntaxHighlighterInstanceInterface interface
public:
	QSyntaxHighlighter *highlighter()
	{
		return( this );
	}

protected:
	virtual void highlightBlock( const QString &pTextBlock ) Q_DECL_OVERRIDE;

private:
	struct HighlightingRule
	{
		QRegExp pattern;
		QTextCharFormat format;
	};

	QVector<HighlightingRule> highlightingRules;

	QVector<HighlightingRule> variableRules;

	QRegExp commentStartExpression;
	QRegExp commentEndExpression;

	QTextCharFormat keywordFormat;
	QTextCharFormat variableFormat;
	QTextCharFormat classFormat;
	QTextCharFormat defineFormat;
	QTextCharFormat singleLineCommentFormat;
	QTextCharFormat multiLineCommentFormat;
	QTextCharFormat quotationFormat;
	QTextCharFormat functionFormat;
};


#endif // SYNTAXHIGHLIGHTERLUA_H
