#include "verb.h"
#include "object.h"
#include "lua_moo.h"
#include "lua_object.h"
#include <QDebug>

void Verb::save( QDataStream &pData ) const
{
	Func::save( pData );

	pData << quint16( mDirectObject );
	pData << quint16( mIndirectObject );
	pData << quint16( mPrepositionType );
	pData << mPreposition;
	pData << mAliases;
}

void Verb::load( QDataStream &pData )
{
	quint16		D, I, P;

	Func::load( pData );

	pData >> D;
	pData >> I;
	pData >> P;
	pData >> mPreposition;
	pData >> mAliases;

	mDirectObject    = ArgObj( D );
	mIndirectObject  = ArgObj( I );
	mPrepositionType = ArgObj( P );
}

void Verb::initialise( void )
{
	Func::initialise();

	mDirectObject = Verb::ANY;
	mIndirectObject = Verb::ANY;
	mPrepositionType = Verb::ANY;
}

QStringList Verb::parse( const QString &pInput, QString &pArgStr )
{
	QStringList		Words;
	bool			InQuote = false;
	bool			IsEscaped = false;
	QString			Word;

	pArgStr.clear();

	if( pInput.isEmpty() )
	{
		return( Words );
	}

	switch( pInput.at( 0 ).toLatin1() )
	{
		case '"':
			pArgStr = pInput.mid( 1 );
			Words << "say";
			Words << pArgStr;
			return( Words );

		case ':':
			pArgStr = pInput.mid( 1 );
			Words << "emote";
			Words << pArgStr;
			return( Words );

		case ';':
			pArgStr = pInput.mid( 1 );
			Words << "eval";
			Words << pArgStr;
			return( Words );
	}

	for( int i = 0 ; i < pInput.size() ; i++ )
	{
		QChar		ch = pInput.at( i );

		if( !IsEscaped && ch == '\\' )
		{
			IsEscaped = true;
		}
		else if( !IsEscaped && ch == '"' )
		{
			InQuote = !InQuote;
		}
		else
		{
			if( IsEscaped )
			{
				switch( ch.toLatin1() )
				{
					case '"':
						break;

					default:
						Word.append( "\\" );
						break;
				}

				IsEscaped = false;
			}

			if( !InQuote && ch.isSpace() )
			{
				if( Words.isEmpty() )
				{
					pArgStr = pInput.mid( i + 1 );
				}

				if( !Word.isEmpty() )
				{
					Words << Word;

					Word.clear();
				}
			}
//			else if( !InQuote && ch == '$' )
//			{
//				Word.append( "o(0):" );
//			}
			else
			{
				Word.append( ch );
			}
		}
	}

	if( !Word.isEmpty() )
	{
		Words << Word;
	}
/*
	QStringList		ArgLst;

	for( QStringList::const_iterator it = Words.begin() ; it != Words.end() ; it++ )
	{
		if( it == Words.begin() )
		{
			continue;
		}

		if( it->contains( " " ) )
		{
			ArgLst.append( QString( "\"%1\"" ).arg( *it ) );
		}
		else
		{
			ArgLst.append( *it );
		}
	}

	pArgStr = ArgLst.join( " " );
*/
	return( Words );
}

/*
In the simplest case, a verb-name is just a word made up of any characters
other than spaces and stars (i.e., ` ' and `*').

In this case, the verb-name matches only itself; that is, the name must be
matched exactly.

If the name contains a single star, however, then the name matches any prefix
of itself that is at least as long as the part before the star. For example,
the verb-name `foo*bar' matches any of the strings `foo', `foob', `fooba', or
`foobar'; note that the star itself is not considered part of the name.

If the verb name ends in a star, then it matches any string that begins with
the part before the star. For example, the verb-name `foo*' matches any of the
strings `foo', `foobar', `food', or `foogleman', among many others.

As a special case, if the verb-name is `*' (i.e., a single star all by itself),
then it matches anything at all.
*/

bool Verb::matchName( const QString &pPatternList, const QString &pMatch )
{
	QStringList		Patterns = pPatternList.split( ' ' );

	foreach( const QString &Pattern, Patterns )
	{
		const int	StarIndex = Pattern.indexOf( '*' );

		if( StarIndex < 0 )
		{
			if( Pattern.compare( pMatch, Qt::CaseInsensitive ) == 0 )
			{
				return( true );
			}

			continue;
		}

		if( Pattern.size() == 1 )
		{
			return( true );
		}

		const QString	Prefix = Pattern.left( StarIndex );
		const QString	Suffix = Pattern.mid( StarIndex + 1 );

		//qDebug() << "Match:" << pMatch << "Index:" << StarIndex << "Prefix:" << Prefix << " Suffix:" << Suffix;

		if( !Prefix.isEmpty() && !pMatch.startsWith( Prefix, Qt::CaseInsensitive ) )
		{
			continue;
		}

		if( Suffix.isEmpty() )
		{
			return( true );
		}

		const QString	MatchSuffix = pMatch.mid( Prefix.size() );

		//qDebug() << MatchSuffix;

		if( MatchSuffix.isEmpty() || Suffix.startsWith( MatchSuffix, Qt::CaseInsensitive ) )
		{
			return( true );
		}
	}

	return( false );
}

bool Verb::matchPreposition( const QString &pPreposition )
{
	const QStringList	PrpLst = pPreposition.split( "/" );

	return( PrpLst.contains( mPreposition, Qt::CaseInsensitive ) );
}

bool Verb::matchArgs( ObjectId pObjectId, ObjectId DirectObjectId, const QString &pPreposition, ObjectId IndirectObjectId )
{
	if( mDirectObject == Verb::NONE )
	{
		if( DirectObjectId != -1 )
		{
			return( false );
		}
	}
	else if( mDirectObject == Verb::THIS )
	{
		if( DirectObjectId != pObjectId )
		{
			return( false );
		}
	}
	else if( mDirectObject != Verb::ANY )
	{
		return( false );
	}

	if( mIndirectObject == Verb::NONE )
	{
		if( IndirectObjectId != -1 )
		{
			return( false );
		}
	}
	else if( mIndirectObject == Verb::THIS )
	{
		if( IndirectObjectId != pObjectId )
		{
			return( false );
		}
	}
	else if( mIndirectObject != Verb::ANY )
	{
		return( false );
	}

	if( mPrepositionType == Verb::NONE  )
	{
		if( !pPreposition.isEmpty() )
		{
			return( false );
		}
	}
	else if( mPrepositionType == Verb::THIS )
	{
		if( !matchPreposition( pPreposition ) )
		{
			return( false );
		}
	}

	return( true );
}

void Verb::addAlias( const QString &pAlias )
{
	QStringList		AliasList = mAliases.split( ' ', QString::SkipEmptyParts );

	AliasList.removeAll( pAlias );

	AliasList.append( pAlias );

	mAliases = AliasList.join( " " );
}

void Verb::remAlias( const QString &pAlias )
{
	QStringList		AliasList = mAliases.split( ' ', QString::SkipEmptyParts );

	AliasList.removeAll( pAlias );

	mAliases = AliasList.join( " " );
}

