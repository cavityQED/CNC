#include "syncAction.h"

namespace CNC
{

SyncAction::SyncAction(codeBlock block, QWidget* parent) : Action(block, parent)
{

}

void SyncAction::execute()
{
	for(auto &a : m_actions)
	{
		//a->enable_sync_mode(true);
		//a->execute();
		//a->wait_for_ready();
	}


	//Toggle sync pin
	gpioWrite(m_syncPin, 1);
	gpioWrite(m_syncPin, 0);
}

}