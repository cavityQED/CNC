#ifndef STEPPERACTION_H
#define STEPPERACTION_H

#include "action.h"

namespace CNC
{

class StepperAction : public Action
{
	Q_OBJECT

public:

	StepperAction(std::shared_ptr<CNC::codeBlock> block, QWidget* parent = nullptr);
	~StepperAction() {}

public slots:
	
	virtual void execute() override;

protected:

};

}//CNC namespace

#endif