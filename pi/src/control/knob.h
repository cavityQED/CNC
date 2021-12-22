#ifndef KNOB_H
#define KNOB_H

#include <pigpio.h>
#include <unistd.h>


class Knob : public QWidget
{
	Q_OBJECT

public:

	static void callback(int gpio, int level, uint32_t tick, void* arg)
	{ 
		Knob* knob = (Knob*)arg;
		knob->cur() += gpioRead(knob->pin1());
		knob->cur() += (gpioRead(knob->pin2()) << 1);

		if(knob->cur() == 3 && knob->pre() == 2)
			knob->emit turn(true);
		else if(knob->cur() == 3 && knob->pre() == 1)
			knob->emit turn(false);

		knob->pre() = knob->cur();
		knob->cur() = 0;
	}

	Knob(int pin1, int pin2, QWidget* parent = nullptr)
		: QWidget(parent), m_pin1(pin1), m_pin2(pin2)
	{
		gpioSetMode(m_pin1, PI_INPUT);
		gpioSetMode(m_pin2, PI_INPUT);

		gpioSetPullUpDown(m_pin1, PI_PUD_UP);
		gpioSetPullUpDown(m_pin2, PI_PUD_UP);

		gpioSetISRFuncEx(m_pin1, RISING_EDGE, 0, callback, this);
		gpioSetISRFuncEx(m_pin2, RISING_EDGE, 0, callback, this);
	}

	~Knob() {}

public:

	uint8_t&	cur()	{return m_cur;}
	uint8_t&	pre()	{return m_pre;}
	int			pin1()	{return m_pin1;}
	int			pin2()	{return m_pin2;}

signals:

	void turn(bool cw);

protected:

	uint8_t m_cur = 0;
	uint8_t m_pre = 0;

	int m_pin1;
	int m_pin2;
};

#endif