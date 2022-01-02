#ifndef STEPPERGROUP_H
#define STEPPERGROUP_H

#include "stepperMotor.h"

#include <map>

namespace CNC
{

namespace DEVICE
{

class StepperGroup : public QWidget
{
	Q_OBJECT

public:

	StepperGroup(QWidget* parent = nullptr);
	~StepperGroup() {}

	bool addStepper(CNC::AXIS axis, CNC::DEVICE::stepperMotor* stepper);

	void update();
	void pause();
	void resume();
	void stop();
	void homeAll();
	void executeBlock(const CNC::codeBlock* b);

signals:

	void motorMotion(bool motion);

public:

	bool inMotion()		{return m_motion;}

protected:

	CNC::position_t		m_startPosition;
	CNC::position_t		m_endPosition;
	double				m_feedRate;
	bool				m_motion;

	std::map<CNC::AXIS, CNC::DEVICE::stepperMotor*>		m_steppers;
};

}//DEVICE namespace
}//CNC namespace

#endif