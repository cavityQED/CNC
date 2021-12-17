#ifndef KNOB_H
#define KNOB_H

#include <pigpio.h>
#include <unistd.h>


class Knob : public QWidget
{
	Q_OBJECT

public:

	void callback(int gpio, int level, uint32_t tick)
	{
		m_cur += gpioRead(m_pin1);
		m_cur += (gpioRead(m_pin2) << 1);

		if(m_cur == 3 && m_pre == 2)
			emit cw_turn();
		else if(m_cur == 3 && m_pre == 1)
			emit ccw_turn();

		m_pre = m_cur;
		m_cur = 0;
	}

	Knob(int pin1, int pin2, QWidget* parent = nullptr)
		: QWidget(parent), m_pin1(pin1), m_pin2(pin2)
	{
		gpioSetMode(m_pin1);
		gpioSetMode(m_pin2);

		gpioSetPullUpDown(m_pin1, PI_PUD_UP);
		gpioSetPullUpDown(m_pin2, PI_PUD_UP);

		gpioSetISRFunc(m_pin1, RISING_EDGE, 0, callback);
		gpioSetISRFunc(m_pin2, RISING_EDGE, 0, callback);
	}

	~Knob() {}

signals:

	void cw_turn();
	void ccw_turn();

protected:

	uint8_t m_cur = 0;
	uint8_t m_pre = 0;

	int m_pin1;
	int m_pin2;
};


/*
#define PIN1 17
#define PIN2 27

static int MSB = 0;
static int LSB = 0;

static uint8_t cur = 0;
static uint8_t pre = 3;

static uint8_t mask = 0;
static const uint8_t cw = 0b01001011;
static const uint8_t ccw = 0b10000111;

static void knob_callback(int gpio, int level, uint32_t tick)
{
	cur += gpioRead(PIN1);
	cur += (gpioRead(PIN2) << 1);

	if(cur == 3 && pre == 2)
		std::cout << "clockwise\n";
	else if(cur == 3 && pre == 1)
		std::cout << "counterclockwise\n";

	pre = cur;
	cur = 0;
}

static void knob_setup()
{
	gpioSetMode(PIN1, PI_INPUT);
	gpioSetMode(PIN2, PI_INPUT);

	gpioSetPullUpDown(PIN1, PI_PUD_UP);
	gpioSetPullUpDown(PIN2, PI_PUD_UP);

	gpioSetISRFunc(PIN1, EITHER_EDGE, 0, knob_callback);
	gpioSetISRFunc(PIN2, EITHER_EDGE, 0, knob_callback);
}
*/
#endif