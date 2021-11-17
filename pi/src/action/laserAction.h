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

	LaserAction(std::shared_ptr<CNC::codeBlock> block, QWidget* parent = nullptr);
	~LaserAction() {}

	void setLaser(std::shared_ptr<Laser> l)		{m_laser = l;}

public slots:
	
	virtual void execute() override;

protected:

	std::shared_ptr<Laser>	m_laser = nullptr;
};

}//CNC namespace

#endif