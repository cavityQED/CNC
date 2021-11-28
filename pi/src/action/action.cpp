#include "action.h"

namespace CNC
{

Action::Action(CNC::codeBlock block, QWidget* parent) : QWidget(parent), m_block(std::move(block))
{
	
}

Action::Action(QWidget* parent) : QWidget(parent)
{
	
}

}//CNC namespace