#ifndef LUA_UTILITIES_H
#define LUA_UTILITIES_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <QString>
#include <QMap>
#include <QVariant>
#include <QXmlDefaultHandler>

typedef QMap<QString,lua_CFunction>		LuaMap;

#if LUA_VERSION_NUM < 502
extern void *luaL_testudata( lua_State *L, int ud, const char *tname );
#endif

extern int luaL_pushvariant( lua_State *L, const QVariant &pV );

class XmlOutputParser : private QXmlDefaultHandler
{
public:
	XmlOutputParser( lua_State *L, QString pText )
		: mL( L )
	{
		mSRC.setData( "<moo>" + pText + "</moo>" );

		QXmlSimpleReader	Reader;

		Reader.setContentHandler( this );
		Reader.setEntityResolver( this );
		Reader.setErrorHandler( this );

		Reader.parse( mSRC );
	}

	QString result( void ) const
	{
		return( mXML );
	}

	// QXmlContentHandler interface
public:
	virtual bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts) Q_DECL_OVERRIDE;
	virtual bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) Q_DECL_OVERRIDE;
	virtual bool characters(const QString &ch) Q_DECL_OVERRIDE;
	virtual bool skippedEntity(const QString &name) Q_DECL_OVERRIDE;
	virtual bool warning(const QXmlParseException &exception) Q_DECL_OVERRIDE;
	virtual bool error(const QXmlParseException &exception) Q_DECL_OVERRIDE;
	virtual bool fatalError(const QXmlParseException &exception) Q_DECL_OVERRIDE;
	virtual QString errorString() const Q_DECL_OVERRIDE;
	virtual bool endDocument() Q_DECL_OVERRIDE;

private:
	typedef struct Element
	{
		QString			mNamespaceUri;
		QString			mLocalName;
		QString			mName;
		QString			mContent;
		QXmlAttributes	mAttrs;
	} Element;

private:
	lua_State			*mL;
	QXmlInputSource		 mSRC;
	QString				 mXML;
	QVector<Element>	 mElementStack;
};

class lua_util
{
public:
	static QString processOutputTags( lua_State *L, QString pText );
};

#endif // LUA_UTILITIES_H
