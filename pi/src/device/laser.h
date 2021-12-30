#ifndef LASER_H
#define LASER_H

#include <pigpio.h>

#include <iostream>

#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QString>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QAction>

#include "device.h"

#define LASER_PIN 		21
#define LASER_OVERRIDE	

namespace CNC
{

namespace DEVICE
{

class Laser : public Device
{
	Q_OBJECT

public:
	Laser(QWidget* parent = nullptr);

	void on();
	void off();

public:

	virtual bool isReady()	{return true;}
	
public slots:
	void increase_power()
	{
		if(m_power < 5)
			m_power += 1;
		else if(m_power <= 95)
			m_power += 5;
		else
			m_power = 100;
		gpioPWM(LASER_PIN, m_power*255/100);
		power_edit->setText(QString::number(m_power, 'f', 1));
	}
	
	void decrease_power()
	{
		if(m_power <= 5 && m_power > 0)
			m_power -= 1;
		else if(m_power > 5)
			m_power -= 5;
		else
			m_power = 0;
		gpioPWM(LASER_PIN, m_power*255/100);
		power_edit->setText(QString::number(m_power, 'f', 1));
	}
	
	void setPower(int pow, bool start = false);
	
protected:
	QPushButton*	increase_button;
	QPushButton*	decrease_button;
	QLineEdit*		power_edit;
	
	int				m_power = 0;	
};

}//DEVICE namespace
}//CNC namespace

#endif