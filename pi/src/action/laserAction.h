#ifndef LASERACTION_H
#define LASERACTION_H

#include "action.h"
#include "device/laser.h"

namespace CNC
{

class LaserAction : public Action
{
	Q_OBJECT

public:

	LaserAction(	Laser* laser,
					CNC::codeBlock block,
					QWidget* parent = nullptr);
	~LaserAction() {}

public slots:
	
	virtual void execute() override;

protected:

	Laser*	m_laser = nullptr;
};

}//CNC namespace

#endif