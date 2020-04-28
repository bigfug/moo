#include "lua_text.h"

#include "objectmanager.h"
#include "lua_moo.h"
#include "lua_object.h"
#include "connection.h"
#include "lua_utilities.h"
#include "mooexception.h"
#include "lua_task.h"
#include "connectionmanager.h"

const luaL_Reg lua_text::mLuaStatic[] =
{
	{ "s", lua_text::luaPronounSubstitution },
	{ "bold", lua_text::luaBold },
	{ "escape", lua_text::luaEscape },
	{ 0, 0 }
};

void lua_text::initialise()
{
	lua_moo::addFunctions( mLuaStatic );
}

void lua_text::luaRegisterState( lua_State *L )
{
	Q_UNUSED( L )
}

int lua_text::luaPronounSubstitution( lua_State *L )
{
	lua_task			*Command = lua_task::luaGetTask( L );

	try
	{
		const char			*S = luaL_checkstring( L, 1 );

		QString				 D;
		bool				 E = false;
		QString				 SubStr;

		Object				*O = nullptr;
		Object				*Player = nullptr;
		Object				*Location = nullptr;
		Object				*Direct = nullptr;
		Object				*Indirect = nullptr;

		while( *S )
		{
			const char		 c = *S++;

			if( E )
			{
				if( SubStr.isEmpty() )
				{
					SubStr += c;

					if( c != '(' && c != '[' )
					{
						E = false;
					}
				}
				else if( SubStr.startsWith( '(' ) )
				{
					SubStr += c;

					if( c == ')' )
					{
						E = false;
					}
				}
				else if( SubStr.startsWith( '[' ) )
				{
					SubStr += c;

					if( c == ']' )
					{
						E = false;
					}
				}

				if( !E && !SubStr.isEmpty() )
				{
					QString				 SubRes;

					if( SubStr.size() == 1 )
					{
						QString				 SubLwr = SubStr.toLower();

						if( SubLwr == QStringLiteral( "n" ) )
						{
							if( ( Player = ( Player ? Player : ObjectManager::o( Command->task().player() ) ) ) )
							{
								SubRes = Player->name();
							}
						}
						else if( SubLwr == QStringLiteral( "o" ) )
						{
							if( ( O = ( O ? O : ObjectManager::o( Command->task().object() ) ) ) )
							{
								SubRes = O->name();
							}
						}
						else if( SubLwr == QStringLiteral( "d" ) )
						{
							if( Command->task().directObjectId() != OBJECT_NONE )
							{
								if( ( Direct = ( Direct ? Direct : ObjectManager::o( Command->task().directObjectId() ) ) ) )
								{
									SubRes = Direct->name();
								}
							}
							else
							{
								SubRes = Command->task().directObjectName();
							}
						}
						else if( SubLwr == QStringLiteral( "i" ) )
						{
							if( Command->task().indirectObjectId() != OBJECT_NONE )
							{
								if( ( Indirect = ( Indirect ? Indirect : ObjectManager::o( Command->task().indirectObjectId() ) ) ) )
								{
									SubRes = Indirect->name();
								}
							}
							else
							{
								SubRes = Command->task().indirectObjectName();
							}
						}
						else if( SubLwr == QStringLiteral( "l" ) )
						{
							if( !Location )
							{
								if( ( Player = ( Player ? Player : ObjectManager::o( Command->task().player() ) ) ) )
								{
									Location = ObjectManager::o( Player->location() );
								}
							}

							if( Location )
							{
								SubRes = Location->name();
							}
						}
						else
						{
							D += "%" + SubStr;
						}
					}
					else if( SubStr.startsWith( '[' ) && SubStr.endsWith( ']' ) )
					{

					}
					else if( SubStr.startsWith( '(' ) && SubStr.endsWith( ')' ) )
					{

					}
					else
					{
						D += "%" + SubStr;
					}

					D += SubRes;

					SubStr.clear();
				}
			}
			else if( c == '%' )
			{
				if( E )
				{
					D += '%';
				}
				else
				{
					E = true;
				}
			}
			else
			{
				D += c;
			}
		}

		if( !SubStr.isEmpty() )
		{
			D += "%" + SubStr;
		}

		lua_pushstring( L, D.toLatin1().constData() );

		return( 1 );
	}
	catch( const mooException &e )
	{
		Command->setException( e );
	}
	catch( ... )
	{

	}

	return( Command->lua_pushexception() );
}

int lua_text::luaBold( lua_State *L )
{
	const char			*S = luaL_checkstring( L, 1 );

	lua_pushstring( L, QString( "<b>%1</b>" ).arg( S ).toLatin1().constData() );

	return( 1 );
}

int lua_text::luaEscape( lua_State *L )
{
	const char			*S = luaL_checkstring( L, 1 );

	lua_pushstring( L, QString::fromLatin1( S ).toHtmlEscaped().toLatin1().constData() );

	return( 1 );
}

QString lua_text::XmlOutputParser::preprocessString( const QString &S )
{
	QString		O;
	QString		E;
	bool		Escaped = false;

	for( QChar C : S )
	{
		if( Escaped )
		{
			if( E.isEmpty() )
			{
				if( C == 'a' )
				{
					O.append( '\a' ); Escaped = false; continue;
				}

				if( C == 'b' )
				{
					O.append( '\b' ); Escaped = false; continue;
				}

				if( C == 'e' )
				{
					O.append( 0x1b ); Escaped = false; continue;
				}

				if( C == 'f' )
				{
					O.append( '\f' ); Escaped = false; continue;
				}

				if( C == 'n' )
				{
					O.append( '\n' ); Escaped = false; continue;
				}

				if( C == 'r' )
				{
					O.append( '\r' ); Escaped = false; continue;
				}

				if( C == 't' )
				{
					O.append( '\t' ); Escaped = false; continue;
				}

				if( C == '\'' )
				{
					O.append( '\'' ); Escaped = false; continue;
				}

				if( C == '\\' )
				{
					O.append( '\\' ); Escaped = false; continue;
				}

				if( C == '"' )
				{
					O.append( '"' ); Escaped = false; continue;
				}

				if( C == 'x' )
				{
					E = C; continue;
				}

				if( C.isDigit() )
				{
					E = C; continue;
				}

				O.append( '\\' );
				O.append( C );

				Escaped = false;

				continue;
			}

			if( E[ 0 ] == 'x' )
			{
				QChar	c = C.toLower();

				if( c >= 'a' && c <= 'f' )
				{
					E.append( c ); continue;
				}
				else
				{
					QChar	V;
					bool	ok;

					E.remove( 0, 1 );

					V = E.toUInt( &ok, 16 );

					if( ok )
					{
						O.append( V );
					}
				}
			}
			else if( E[ 0 ].isDigit() )
			{
				if( C.isDigit() )
				{
					E.append( C ); continue;
				}
				else
				{
					QChar	V;
					bool	ok;

					V = E.toUInt( &ok, 8 );

					if( ok )
					{
						O.append( V );
					}
				}
			}
		}

		if( C == '\\' )
		{
			Escaped = true;

			E.clear();
		}
		else
		{
			O.append( C );
		}
	}

	return( O );
}

QString lua_text::processOutputTags( lua_State *L, QString pText )
{
	return( XmlOutputParser( L, pText ).result() );
}

bool lua_text::XmlOutputParser::startElement( const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts )
{
	Q_UNUSED( namespaceURI )
	Q_UNUSED( qName )
	Q_UNUSED( atts )

	Element		E;

	E.mNamespaceUri = namespaceURI;
	E.mLocalName    = localName;
	E.mName         = qName;
	E.mAttrs        = atts;

	QStringList		NameList = qName.split( ':', QString::SkipEmptyParts );

	if( NameList.size() == 2 )
	{
		const QString		&ObjectName = NameList.at( 0 );

		lua_task		*T = lua_task::luaGetTask( mL );

		Object			*O = Q_NULLPTR;

		if( !ObjectName.compare( "player" ) )
		{
			O = ObjectManager::o( T->task().player() );
		}
		else if( !ObjectName.compare( "object" ) )
		{
			O = ObjectManager::o( T->task().object() );
		}
		else if( !ObjectName.compare( "direct" ) )
		{
			O = ObjectManager::o( T->task().directObjectId() );
		}
		else if( !ObjectName.compare( "indirect" ) )
		{
			O = ObjectManager::o( T->task().indirectObjectId() );
		}
		else if( !ObjectName.compare( "location" ) )
		{
			O = ObjectManager::o( T->task().player() );

			if( O )
			{
				O = ObjectManager::o( O->location() );
			}
		}

		if( O )
		{
			if( !localName.compare( "name" ) )
			{
				E.mContent = O->name();
			}
			else
			{
				Verb			*V = O->verb( localName );

				if( V )
				{
					int r = T->verbCall( V );

					if( r > 0 )
					{
						if( lua_isstring( T->L(), -1 ) )
						{
							const char *r = lua_tostring( T->L(), -1 );

							E.mContent = QString::fromLatin1( r );
						}

						lua_pop( T->L(), r );
					}
				}
				else
				{
					Property	*P = O->prop( localName );

					if( P )
					{
						if( P->type() == QVariant::String )
						{
							E.mContent = P->value().toString();
						}
						else if( P->type() == QVariant::Double )
						{
							E.mContent = QString::number( P->value().toDouble() );
						}
						else if( P->type() == QVariant::Bool )
						{
							E.mContent = ( P->value().toBool() ? "true" : "false" );
						}
						else
						{
							E.mContent = P->value().toString();
						}
					}
				}
			}
		}
	}
	else
	{
		QString			StyleName      = QString( "style/%1" ).arg( qName );
		QString			StyleStartName = QString( "%1.start" ).arg( StyleName );
		QString			Style;

		if( mSettings.contains( StyleStartName ) )
		{
			Style = mSettings.value( StyleStartName ).toString();
		}
		else if( mSettings.contains( StyleName ) )
		{
			Style = mSettings.value( StyleName ).toString();
		}

		E.mContent = preprocessString( Style );
	}

	mElementStack.push_back( E );

	return( true );
}

bool lua_text::XmlOutputParser::endElement( const QString &namespaceURI, const QString &localName, const QString &qName )
{
	Q_UNUSED( namespaceURI )
	Q_UNUSED( localName )
	Q_UNUSED( qName )

	Element		E = mElementStack.takeLast();

	QString		C = E.mContent;

	QString			StyleName = QString( "style/%1.end" ).arg( qName );

	if( mSettings.contains( StyleName ) )
	{
		QString			Style = mSettings.value( StyleName ).toString();

		C.append( preprocessString( Style ) );
	}

	if( mElementStack.isEmpty() )
	{
		mXML.append( C );
	}
	else
	{
		Element		&E2 = mElementStack.last();

		E2.mContent.append( C );
	}

	return( true );
}

QString lua_text::XmlOutputParser::errorString() const
{
	return( QString() );
}

bool lua_text::XmlOutputParser::characters(const QString &ch)
{
	Element		&E = mElementStack.last();

	E.mContent.append( ch );

	return( true );
}

bool lua_text::XmlOutputParser::skippedEntity(const QString &name)
{
	mXML.append( name );

	return( true );
}

bool lua_text::XmlOutputParser::warning(const QXmlParseException &exception)
{
	mXML.append( QString( "\x1b[0m\nWARNING (%1):" ).arg( exception.columnNumber() ) );
	mXML.append( exception.message() );

	return( false );
}

bool lua_text::XmlOutputParser::error(const QXmlParseException &exception)
{
	mXML.append( QString( "\x1b[0m\nERROR (%1):" ).arg( exception.columnNumber() ) );
	mXML.append( exception.message() );

	return( false );
}

bool lua_text::XmlOutputParser::fatalError(const QXmlParseException &exception)
{
	mXML.clear();
	mXML.append( QString( "\r\x1b[0m%1\n" ).arg( mSRC.data() ) );
	mXML.append( QString( ' ' ).repeated( exception.columnNumber() ).append( "^\n" ) );
	mXML.append( QString( "FATAL (%1):" ).arg( exception.columnNumber() ) );
	mXML.append( exception.message().trimmed() );

	return( false );
}

bool lua_text::XmlOutputParser::endDocument()
{
	return( true );
}

QString lua_text::localeString( lua_State *L, int pKeyIdx, Object *pObject, QString pDefault )
{
	QString				 Msg;

	if( lua_isstring( L, pKeyIdx ) )
	{
		Msg = QString( lua_tostring( L, pKeyIdx ) );
	}
	else if( lua_istable( L, pKeyIdx ) )
	{
		QString		LocaleKey;
		int			r1 = 0;
		int			t = lua_gettop( L );

		if( !r1 && pObject )
		{
			LocaleKey = pObject->propValue( "locale" ).toString();

			if( !LocaleKey.isEmpty() )
			{
				lua_getfield( L, pKeyIdx, LocaleKey.toLatin1().constData() );

				r1 = lua_gettop( L ) - t;
			}
		}

		if( !r1 && LocaleKey != "en" )
		{
			LocaleKey = QLocale::system().bcp47Name().toLatin1().constData();

			lua_getfield( L, pKeyIdx, LocaleKey.toLatin1().constData() );

			r1 = lua_gettop( L ) - t;
		}

		if( !r1 && LocaleKey != "en" )
		{
			LocaleKey = "en";

			lua_getfield( L, pKeyIdx, LocaleKey.toLatin1().constData() );

			r1 = lua_gettop( L ) - t;
		}

		if( r1 )
		{
			Msg = QString( lua_tostring( L, -1 ) );

			lua_pop( L, 1 );
		}
		else
		{
			Msg = pDefault;
		}
	}

	return( Msg );
}

QString lua_text::processTextArgs( lua_State *L, int pKeyIdx, QString pString )
{
	for( int i = pKeyIdx ; i <= lua_gettop( L ) ; i++ )
	{
		switch( lua_type( L, i ) )
		{
			case LUA_TNUMBER:
				pString = pString.arg( double( lua_tonumber( L, i ) ) );
				break;

			case LUA_TSTRING:
				pString = pString.arg( QString( lua_tostring( L, i ) ) );
				break;

			case LUA_TBOOLEAN:
				pString = pString.arg( bool( lua_toboolean( L, i ) ) );
				break;
		}
	}

	return( pString );
}

QString lua_text::processString( lua_State *L, Object *O, int pKeyIdx )
{
	QString				 Msg;

	Msg = localeString( L, pKeyIdx, O );

	Msg = processTextArgs( L, pKeyIdx + 1, Msg );

	Msg = processOutputTags( L, Msg );

	return( Msg );
}
