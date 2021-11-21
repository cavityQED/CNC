#ifndef SYNCACTION_H
#define SYNCACTION_H

#include "action.h"

#include <vector>

namespace CNC
{

class SyncAction : public Action
{
	Q_OBJECT

public:

	SyncAction(codeBlock block, QWidget* parent = nullptr);
	~SyncAction() {}

	void clear() {m_actions.clear();}

	void addAction	(Action* a)	{m_actions.push_back(a);}
	void setSyncPin	(int pin)	{m_syncPin = pin;}

public slots:

	virtual void execute() override;

protected:
	std::vector<Action*>	m_actions;
	int						m_syncPin;	//Pin to bring high to signal devices to execute action
};

}//CNC namespace

#endif