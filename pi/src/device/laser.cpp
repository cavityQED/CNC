#include "laser.h"

namespace CNC
{

namespace DEVICE
{

Laser::Laser(QWidget* parent) : Device(parent)
{
	m_deviceType = CNC::DEVICE_TYPE::LASER;
	
	gpioInitialise();
	
	gpioSetMode(LASER_PIN, PI_OUTPUT);
	gpioSetPWMfrequency(LASER_PIN, 5000);
	
	increase_button = new QPushButton(QChar(0x25B2));
	decrease_button = new QPushButton(QChar(0x25BC));
	power_edit = new QLineEdit();
	power_edit->setReadOnly(true);
	power_edit->setText(QString::number(m_power, 'f', 1));
	
	QLabel* label = new QLabel("Laser Power:");
	
	QHBoxLayout* hbox = new QHBoxLayout();
	hbox->addWidget(label);
	hbox->addWidget(power_edit);

	QVBoxLayout* vbox = new QVBoxLayout();
	vbox->addWidget(increase_button);
	vbox->addWidget(decrease_button);
	hbox->addLayout(vbox);

	QVBoxLayout* onoff = new QVBoxLayout();
	QPushButton* on_button = new QPushButton("ON");
	QPushButton* off_button = new QPushButton("OFF");
	
	onoff->addWidget(on_button);
	onoff->addWidget(off_button);
	hbox->addLayout(onoff);

	setLayout(hbox);
	
	connect(increase_button, &QPushButton::released, this, &Laser::increase_power);
	connect(decrease_button, &QPushButton::released, this, &Laser::decrease_power);
	connect(on_button, &QPushButton::released, this, &Laser::on);
	connect(off_button, &QPushButton::released, this, &Laser::off);

	QAction* up = new QAction(this);
	up->setShortcut(Qt::Key_Plus);
	connect(up, &QAction::triggered, this, &Laser::increase_power);
	addAction(up);

	QAction* down = new QAction(this);
	down->setShortcut(Qt::Key_Minus);
	connect(down, &QAction::triggered, this, &Laser::decrease_power);
	addAction(down);
}

void Laser::setPower(const int pow, const bool start)
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

void Laser::executeBlock(const CNC::codeBlock* b)
{
	switch(b->m_numberCode)
	{
		case 0:
			off();
			break;

		case 1:
			on();
			break;

		case 2:
			setPower(b->args().at('P'), true);
			break;

		default:
			break;
	}
}

}//DEVICE namespace
}//CNC namespace