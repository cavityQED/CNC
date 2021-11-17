#ifndef SYNCACTION_H
#define SYNCACTION_H

#include "action.h"

#include <vector>

namespace CNC
{

class syncAction : public Action
{
	Q_OBJECT

public:

	syncAction(codeBlock block, QWidget* parent = nullptr);
	~syncAction() {}

public slots:

	virtual void execute() override;

protected:
	std::vector<Action*>	m_actions;
};

}//CNC namespace

#endif