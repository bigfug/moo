#ifndef MOOAPPLICATION_H
#define MOOAPPLICATION_H

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QVector>

class mooApp;
class OSC;
class ListenerServer;

class MooApplication : public QObject
{
	Q_OBJECT

public:
	MooApplication( QCoreApplication &a );

	virtual ~MooApplication( void );

	inline QCommandLineParser &parser( void )
	{
		return( mParser );
	}

	inline QString optionDataDirectory( void ) const
	{
		return( mParser.value( mOptionDataDirectory ) );
	}

	inline QString optionConfigurationFile( void ) const
	{
		return( mParser.value( mOptionConfiguration ) );
	}

	void process( void );

	bool initialiseApp( void );

	void deinitialiseApp( void );

	static QString sharedDataPath( void );

signals:
	void message( QString pMessage );

private:
	static void staticMessageHandler( QtMsgType type, const QMessageLogContext &context, const QString &msg );

	void messageHandler( QtMsgType type, const QMessageLogContext &context, const QString &msg );

private slots:
	void fileMessageHandler( QString pMessage );

	void debugMessageHandler( QString pMessage );

private:
	static MooApplication	*mInstance;

	QCoreApplication			&mApp;
	mooApp						*mMooApp;
	OSC							*mOSC;
	QVector<ListenerServer *>	 mListenerServers;

	QCommandLineParser			 mParser;
	QCommandLineOption			 mOptionDataDirectory;
	QCommandLineOption			 mOptionConfiguration;
};

#endif // MOOAPPLICATION_H
