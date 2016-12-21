#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QSerialPort>

class SerialPort
{
public:
	SerialPort();

protected:
	QSerialPort			 mSerialPort;
};

#endif // SERIALPORT_H
