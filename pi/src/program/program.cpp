#include "program.h"

namespace CNC
{

Program::Program(QWidget* parent)
	: QWidget(parent)
{

}

Program::Program(const std::string filename, QWidget* parent)
	: QWidget(parent), m_filename(filename)
{

}

void Program::timerEvent(QTimerEvent* e)
{
	m_programMotion = false;



	if(!m_programMotion)
	{
		killTimer(e->timerId());
		execute_next();
	}
}

void Program::start()
{
	reset();
	m_timer = startTimer(m_programTimerPeriod);

}

void Program::stop()
{

}

void Program::pause()
{
	killTimer(m_timer);
}

void Program::resume()
{
	m_timer = startTimer(m_programTimerPeriod);
}

void Program::reset()
{
	stop();
	m_programStep = 0;
}

void Program::execute_next()
{
	if(m_programStep < m_programBlocks.size())
	{
		//execute next code block
	}

	else
		stop();
}

void Program::save()
{

}

void Program::load()
{
	m_codeFile.open(m_filename);	//Open the code text file

	if(!m_codeFile.is_open())		//If file not open, there's nothing to do, return 
		return;						//TODO - Alert user

	m_fileContents.clear();

	std::stringstream	buf;
	buf << m_codeFile.rdbuf();
	loadBlocks(buf.str());

}

void Program::load(const std::string& codeFileContents)
{
	m_fileContents.clear();
	m_fileContents.append(codeFileContents.c_str());

	//If file opened successfully, clear the program to load fresh
	m_programBlocks.clear();

	CNC::codeBlock					tmpBlock {};	//Temporary code block
	std::string::const_iterator		code_iterator;	//Iterator

	//Previous absolute positions of the axes
	double prev_x = 0;
	double prev_y = 0;
	double prev_z = 0;

		//Reset the iterator to the beginning of the line
	code_iterator = codeFileContents.begin();

		//Iterate through the line, creating codeBlocks as necessary
	while(code_iterator != codeFileContents.end())
	{
		//Check if we're at the start of a supported letter code
		if(is_supported_letter_code(*code_iterator))
		{
			//If so, reset the temp block
			memset(&tmpBlock, 0, sizeof(CNC::codeBlock));

			//Set capital letter code
			tmpBlock.letterCode = (*code_iterator >= 97)? *code_iterator - 32 : *code_iterator;
			//Set the number code
			tmpBlock.numberCode = get_double(++code_iterator);

			//Set the previous positions
			tmpBlock.prev_x = prev_x;
			tmpBlock.prev_y = prev_y;
			tmpBlock.prev_z = prev_z;

			//Populate codeBlock variables until another supported code letter or the end of the line is reached
			while(!(is_supported_letter_code(*code_iterator)) && code_iterator != codeFileContents.end())
			{
				switch(*code_iterator)
				{
					case 'x': case 'X': tmpBlock.x = get_double(++code_iterator); tmpBlock.abs_x = true; prev_x = tmpBlock.x; break;
					case 'y': case 'Y': tmpBlock.y = get_double(++code_iterator); tmpBlock.abs_y = true; prev_y = tmpBlock.y; break;
					case 'z': case 'Z': tmpBlock.z = get_double(++code_iterator); tmpBlock.abs_z = true; prev_z = tmpBlock.z; break;
				
					case 'u': case 'U': tmpBlock.u = get_double(++code_iterator); tmpBlock.abs_x = false; prev_x += tmpBlock.u; break;
					case 'v': case 'V': tmpBlock.v = get_double(++code_iterator); tmpBlock.abs_y = false; prev_y += tmpBlock.v; break;
					case 'w': case 'W': tmpBlock.w = get_double(++code_iterator); tmpBlock.abs_z = false; prev_z += tmpBlock.w; break;
				
					case 'i': case 'I': tmpBlock.i = get_double(++code_iterator); break;
					case 'j': case 'J': tmpBlock.j = get_double(++code_iterator); break;
					case 'k': case 'K': tmpBlock.k = get_double(++code_iterator); break;
					case 'r': case 'R': tmpBlock.r = get_double(++code_iterator); break;

					case 'p': case 'P': tmpBlock.p = get_double(++code_iterator); break;
					case 'f': case 'F': tmpBlock.f = get_double(++code_iterator); break;
					default: code_iterator++;				
				}
			}

			//Add the block to the program
			m_programBlocks.push_back(tmpBlock);
		}
		//If anything besides a supported letter code, increase the iterator
		else
			code_iterator++;
	}
		
	std::cout << "\n****FILE CONTENTS****\n" << m_fileContents.toStdString() << '\n';
}

/************************
*						*
*	HELPER FUNCTIONS	*
*						*
************************/

double Program::get_double(std::string::const_iterator &s)
{
	std::string d {""};	//Create empty string to store double

	while(*s == ' ')
		s++;

	//Grab characters as long as reading a number
	while((*s >= '0' && *s <= '9') || *s == '.' || *s == '-')
	{
		d += *s;
		s++;
	}

	//Return the converted string or 0 if string is invalid
	return (d == "")? 0 : std::stod(d);
}

bool Program::is_supported_letter_code(const char c)
{
	switch(c)
	{
		case 'G': case 'g':
		case 'M': case 'm':
		case 'L': case 'l':
			return true;
		
		default:
			return false;
	}
}

}//CNC namespace