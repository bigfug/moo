#ifndef LUA_TEXT_H
#define LUA_TEXT_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "lua_utilities.h"

#include <QXmlDefaultHandler>
#include <QXmlAttributes>
#include <QVector>

class lua_text
{
public:
	static QString processOutputTags( lua_State *L, QString pText );

private:
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
		static QString preprocessString( const QString &S );

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

private:
	static void initialise( void );

	static void luaRegisterState( lua_State *L );

	static int luaPronounSubstitution( lua_State *L );

	static int luaBold( lua_State *L );

	static int luaEscape( lua_State *L );


	static const luaL_Reg		 mLuaStatic[];

	friend class lua_moo;
};

#endif // LUA_TEXT_H
