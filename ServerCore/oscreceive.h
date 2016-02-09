#ifndef OSCRECEIVE_H
#define OSCRECEIVE_H

#include <QObject>

class OSCReceive : public QObject
{
	Q_OBJECT

public:
	OSCReceive( quint16 pPort );

private:
	quint16		mPort;
};

#endif // OSCRECEIVE_H
