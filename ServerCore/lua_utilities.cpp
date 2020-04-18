#include "lua_utilities.h"
#include "lua_object.h"
#include "lua_task.h"
#include "objectmanager.h"

#include "verb.h"
#include "property.h"

#if LUA_VERSION_NUM < 502

void *luaL_testudata( lua_State *L, int ud, const char *tname )
{
	void *p = lua_touserdata(L, ud);

	if (p != NULL)
	{  /* value is a userdata? */
		if (lua_getmetatable(L, ud))
		{  /* does it have a metatable? */
			lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */

			if (lua_rawequal(L, -1, -2))
			{  /* does it have the correct mt? */
				lua_pop(L, 2);  /* remove both metatables */
				return p;
			}
		}
	}
	return NULL;  /* to avoid warnings */
}

#endif

int luaL_pushvariant( lua_State *L, const QVariant &pV )
{
	if( strcmp( pV.typeName(), "lua_object::luaHandle" ) == 0 )
	{
		lua_object::luaHandle		H = pV.value<lua_object::luaHandle>();

		lua_object::lua_pushobjectid( L, H.O );

		return( 1 );
	}

	switch( pV.type() )
	{
		case QVariant::String:
			lua_pushstring( L, pV.toString().toLatin1() );
			return( 1 );

		case QVariant::Bool:
			lua_pushboolean( L, pV.toBool() );
			return( 1 );

		case QVariant::Double:
			lua_pushnumber( L, pV.toDouble() );
			return( 1 );

		case QVariant::Int:
			lua_pushinteger( L, pV.toInt() );
			return( 1 );

		case QVariant::Map:
			{
				const QVariantMap		&VarMap = pV.toMap();

				lua_newtable( L );

				for( QVariantMap::const_iterator it = VarMap.constBegin() ; it != VarMap.constEnd() ; it++ )
				{
					QString		K = it.key();
					bool		B;
					int			V = K.toInt( &B );

					if( B )
					{
						lua_pushinteger( L, V );
					}
					else
					{
						lua_pushstring( L, K.toLatin1().constData() );
					}

					luaL_pushvariant( L, *it );
					lua_settable( L, -3 );
				}

				return( 1 );
			}

		default:
			lua_pushnil( L );
			break;
	}

	return( 1 );
}

QString lua_util::processOutputTags( lua_State *L, QString pText )
{
	return( XmlOutputParser( L, pText ).result() );
}

bool XmlOutputParser::startElement( const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts )
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
					int r = T->verbCall( O->id(), V );

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

	mElementStack.push_back( E );

	return( true );
}

bool XmlOutputParser::endElement( const QString &namespaceURI, const QString &localName, const QString &qName )
{
	Q_UNUSED( namespaceURI )
	Q_UNUSED( localName )
	Q_UNUSED( qName )

	Element		E = mElementStack.takeLast();

	QString		C;

	if( !E.mContent.isEmpty() )
	{
		C = QString( "<%1>%2</%1>" ).arg( E.mLocalName ).arg( E.mContent ).arg( E.mLocalName );
	}
	else
	{
		C = QString( "<%1 />" ).arg( E.mLocalName );
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

QString XmlOutputParser::errorString() const
{
	return( QString() );
}

bool XmlOutputParser::characters(const QString &ch)
{
	Element		&E = mElementStack.last();

	E.mContent.append( ch );

	return( true );
}

bool XmlOutputParser::skippedEntity(const QString &name)
{
	mXML.append( name );

	return( true );
}

bool XmlOutputParser::warning(const QXmlParseException &exception)
{
	mXML.append( QString( "\x1b[0m\nWARNING (%1):" ).arg( exception.columnNumber() ) );
	mXML.append( exception.message() );

	return( false );
}

bool XmlOutputParser::error(const QXmlParseException &exception)
{
	mXML.append( QString( "\x1b[0m\nERROR (%1):" ).arg( exception.columnNumber() ) );
	mXML.append( exception.message() );

	return( false );
}

bool XmlOutputParser::fatalError(const QXmlParseException &exception)
{
	mXML.append( QString( "\x1b[0m\nFATAL (%1):" ).arg( exception.columnNumber() ) );
	mXML.append( exception.message() );

	return( false );
}

bool XmlOutputParser::endDocument()
{
	return( true );
}
