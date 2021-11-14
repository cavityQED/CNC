#include "Laser.h"

Laser::Laser(QWidget* parent) : QGroupBox(parent)
{
	//gpioInitialise();
	
	gpioSetPWMfrequency(LASER_PIN, 5000);
	
	increase_button = new QPushButton(QChar(0x25B2));
	decrease_button = new QPushButton(QChar(0x25BC));
	power_edit = new QLineEdit();
	power_edit->setReadOnly(true);
	power_edit->setText(QString::number(m_power, 'f', 1));
	
	QLabel* label = new QLabel("Laser Power:");
	
	QHBoxLayout* hbox = new QHBoxLayout(this);
	hbox->addWidget(label);
	hbox->addWidget(power_edit);
	hbox->addWidget(decrease_button);
	hbox->addWidget(increase_button);
	setLayout(hbox);
	
	connect(increase_button, &QPushButton::released, this, &Laser::increase_power);
	connect(decrease_button, &QPushButton::released, this, &Laser::decrease_power);
}

void Laser::setPower(int pow, bool start)
{
	m_power = pow;
	power_edit->setText(QString::number(m_power, 'f', 1));

	if(start)
		on();
}

void Laser::on()
{
	gpioPWM(LASER_PIN, m_power*255/100);
}

void Laser::off()
{
	gpioPWM(LASER_PIN, 0);
}
