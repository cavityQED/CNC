#ifndef DEVICE_H
#define DEVICE_H

#include "common.h"

namespace CNC
{

enum DEVICE_TYPE
{
	LASER,
	STEPPER_MOTOR,
};

class Device : public QWidget
{
	Q_OBJECT

public:

	Device(QWidget* parent = nullptr);
	~Device() {}

public:

	virtual bool isReady() = 0;

protected:

	CNC::DEVICE_TYPE	m_deviceType;
};

}//CNC namespace

#endif