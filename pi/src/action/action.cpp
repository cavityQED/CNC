#include "action.h"

namespace CNC
{

Action::Action(std::shared_ptr<CNC::codeBlock> block, QWidget* parent) : QWidget(parent), m_block(block)
{
	
}

}//CNC namespace