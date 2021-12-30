#include "codeBlock.h"

namespace CNC
{

codeBlock::codeBlock(QObject* parent) : QObject(parent)
{

}

void codeBlock::clear()
{
	m_letterCode = ' ';
	m_numberCode = -1;

	m_args.clear();
}

bool codeBlock::addArg(char c, double d)
{
	auto pair = m_args.insert(std::make_pair(c, d));
	return pair.second;
}

std::ostream& operator<<(std::ostream& out, const codeBlock* block)
{
	out << block->m_letterCode << block->m_numberCode << ":\n";  
	for(auto arg : block->args())
		out << "\t[" << arg.first << ", " << arg.second << "]\n"; 
	return out;
}

}//CNC namespace