#ifndef LASER_H
#define LASER_H

#include <pigpio.h>

#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QString>
#include <QHBoxLayout>
#include <QGroupBox>

#define LASER_PIN 21

class Laser : public QGroupBox
{
public:
	Laser(QWidget* parent = nullptr);
	
public slots:
	void increase_power()
	{
		if(m_power <= 95)
			m_power += 5;
		else
			m_power = 100;
		gpioPWM(LASER_PIN, m_power*255/100);
		power_edit->setText(QString::number(m_power, 'f', 1));
	}
	
	void decrease_power()
	{
		if(m_power >= 5)
			m_power -= 5;
		else
			m_power = 0;
		gpioPWM(LASER_PIN, m_power*255/100);
		power_edit->setText(QString::number(m_power, 'f', 1));
	}
	
protected:
	QPushButton*	increase_button;
	QPushButton*	decrease_button;
	QLineEdit*		power_edit;
	
	int				m_power = 0;	
};

#endif
