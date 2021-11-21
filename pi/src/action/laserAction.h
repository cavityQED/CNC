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

	LaserAction(	CNC::DEVICE::Laser* laser,
					CNC::codeBlock block,
					QWidget* parent = nullptr);
	~LaserAction() {}

	void setLaser(CNC::DEVICE::Laser* laser)	{m_laser = laser;}

public slots:
	
	virtual void execute() override;

protected:

	CNC::DEVICE::Laser*		m_laser = nullptr;
};

}//CNC namespace

#endif