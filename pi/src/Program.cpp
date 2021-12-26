#include "Program.h"

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

void Program::printBlocks() const
{
	for(auto &b : m_programBlocks)
	{
		std::cout << b.letterCode << b.numberCode << ":\n";
		std::cout << "\t[x, y, z]: - " << b.x << ", " << b.y << ", " << b.z << '\n';
		std::cout << "\t[u, v, w]: - " << b.u << ", " << b.v << ", " << b.w << '\n';
		std::cout << "\t[i, j, k]: - " << b.i << ", " << b.j << ", " << b.k << '\n';
		std::cout << "\t[r, p, f]: - " << b.r << ", " << b.p << ", " << b.f << '\n';

	}
}

void Program::timerEvent(QTimerEvent* e)
{
	m_programMotion = false;

	if(m_devices.x_axis != nullptr)
	{
		m_devices.x_axis->esp_receive();
		m_programMotion = m_programMotion || m_devices.x_axis->isMoving();
	}

	if(m_devices.y_axis != nullptr)
	{
		m_devices.y_axis->esp_receive();
		m_programMotion = m_programMotion || m_devices.y_axis->isMoving();
	}

	if(m_devices.z_axis != nullptr)
	{
		m_devices.z_axis->esp_receive();
		m_programMotion = m_programMotion || m_devices.z_axis->isMoving();
	}

	if(!m_programMotion)
	{
		killTimer(e->timerId());
		execute_next();
	}
}

void Program::execute_next()
{
	if(m_programStep < m_programActions.size())
	{
		std::cout << "\n********STEP " << m_programStep << "********\n";
		m_programActions[m_programStep++]->execute();
		m_timer = startTimer(m_programTimerPeriod);
	}

	else
		stop();
}

void Program::start()
{
	reset();
	m_timer = startTimer(m_programTimerPeriod);

}

void Program::pause()
{
	if(m_devices.x_axis)
		m_devices.x_axis->esp_timer_pause(true);
	if(m_devices.y_axis)
		m_devices.y_axis->esp_timer_pause(true);
	killTimer(m_timer);
}

void Program::resume()
{
	if(m_devices.x_axis)
		m_devices.x_axis->esp_timer_pause(false);
	if(m_devices.y_axis)
		m_devices.y_axis->esp_timer_pause(false);
	m_timer = startTimer(m_programTimerPeriod);
}

void Program::stop()
{
	if(m_devices.x_axis != nullptr)
		m_devices.x_axis->esp_stop();
	if(m_devices.y_axis != nullptr)
		m_devices.y_axis->esp_stop();
	if(m_devices.z_axis != nullptr)
		m_devices.z_axis->esp_stop();
	if(m_devices.laser != nullptr)
		m_devices.laser->off();
}

void Program::reset()
{
	stop();
	m_programStep = 0;
}

void Program::save()
{

}

void Program::load()
{
	loadBlocks();
	loadActions();
}

void Program::load(const std::string& codeFileContents)
{
	loadBlocks(codeFileContents);
	loadActions();
}

void Program::loadBlocks()
{
	m_codeFile.open(m_filename);	//Open the code text file

	if(!m_codeFile.is_open())		//If file not open, there's nothing to do, return 
		return;						//TODO - Alert user

	m_fileContents.clear();

	std::stringstream	buf;
	buf << m_codeFile.rdbuf();
	loadBlocks(buf.str());

}//loadBlocks

void Program::loadBlocks(const std::string& codeFileContents)
{
	m_fileContents.clear();
	m_fileContents.append(codeFileContents.c_str());

	//If file opened successfully, clear the program to load fresh
	m_programBlocks.clear();

	CNC::codeBlock				tmpBlock {};	//Temporary code block
	std::string::const_iterator	line_it;		//Text line iterator

	//Previous absolute positions of the axes
	double prev_x = 0;
	double prev_y = 0;
	double prev_z = 0;

		//Reset the iterator to the beginning of the line
	line_it = codeFileContents.begin();

		//Iterate through the line, creating codeBlocks as necessary
	while(line_it != codeFileContents.end())
	{
		//Check if we're at the start of a supported letter code
		if(is_supported_letter_code(*line_it))
		{
			//If so, reset the temp block
			memset(&tmpBlock, 0, sizeof(CNC::codeBlock));

			//Set capital letter code
			tmpBlock.letterCode = (*line_it >= 97)? *line_it - 32 : *line_it;
			//Set the number code
			tmpBlock.numberCode = get_double(++line_it);

			//Set the previous positions
			tmpBlock.prev_x = prev_x;
			tmpBlock.prev_y = prev_y;
			tmpBlock.prev_z = prev_z;

			//Populate codeBlock variables until another supported code letter or the end of the line is reached
			while(!(is_supported_letter_code(*line_it)) && line_it != codeFileContents.end())
			{
				switch(*line_it)
				{
					case 'x': case 'X': tmpBlock.x = get_double(++line_it); tmpBlock.abs_x = true; prev_x = tmpBlock.x; break;
					case 'y': case 'Y': tmpBlock.y = get_double(++line_it); tmpBlock.abs_y = true; prev_y = tmpBlock.y; break;
					case 'z': case 'Z': tmpBlock.z = get_double(++line_it); tmpBlock.abs_z = true; prev_z = tmpBlock.z; break;
				
					case 'u': case 'U': tmpBlock.u = get_double(++line_it); tmpBlock.abs_x = false; prev_x += tmpBlock.u; break;
					case 'v': case 'V': tmpBlock.v = get_double(++line_it); tmpBlock.abs_y = false; prev_y += tmpBlock.v; break;
					case 'w': case 'W': tmpBlock.w = get_double(++line_it); tmpBlock.abs_z = false; prev_z += tmpBlock.w; break;
				
					case 'i': case 'I': tmpBlock.i = get_double(++line_it); break;
					case 'j': case 'J': tmpBlock.j = get_double(++line_it); break;
					case 'k': case 'K': tmpBlock.k = get_double(++line_it); break;
					case 'r': case 'R': tmpBlock.r = get_double(++line_it); break;

					case 'p': case 'P': tmpBlock.p = get_double(++line_it); break;
					case 'f': case 'F': tmpBlock.f = get_double(++line_it); break;
					default: line_it++;				
				}
			}

			//Add the block to the program
			m_programBlocks.push_back(tmpBlock);
		}
		//If anything besides a supported letter code, increase the iterator
		else
			line_it++;
	}
		
	std::cout << "\n****FILE CONTENTS****\n" << m_fileContents.toStdString() << '\n';

}

void Program::loadActions()
{
	m_programActions.clear();

	double feed_rate = 0;

	CNC::StepperAction::axes_t	axes {m_devices.x_axis, m_devices.y_axis, m_devices.z_axis};

	for(auto &b : m_programBlocks)
	{

		switch(b.letterCode)
		{
			case 'G':
			{
				if(b.f)
					feed_rate = b.f;
				else
					b.f = feed_rate;

				switch(b.numberCode)
				{
					case 0:
					{	//Rapid Positioning
						std::cout << "\n\nAdding G0 Rapid Move\n";
						m_programActions.push_back(new StepperAction(b, axes));
						std::cout << "Added G0 Move\n";
						break;
					}

					case 1:
					{	//Linear Interpolation
						std::cout << "\n\nAdding G1 Linear Move\n";
						m_programActions.push_back(new StepperAction(b, axes));
						std::cout << "Added G1 Move\n";
						break;
					}

					case 2:
					{	//Circular Interpolation CW
						std::cout << "\n\nAdding G2 CW Circular Move\n";
						m_programActions.push_back(new StepperAction(b, axes));
						std::cout << "Added G2 Move\n";
						break;
					}

					case 3:
					{	//Circular Interpolation CW
						std::cout << "\n\nAdding G3 CCW Circular Move\n";
						m_programActions.push_back(new StepperAction(b, axes));
						std::cout << "Added G3 Move\n";
						break;
					}

					default:
						break;
				}

				break;
			}

			case 'L':
			{
				std::cout << "\n\nAdding laser action to program actions\n";
				m_programActions.push_back(new CNC::LaserAction(m_devices.laser, b));
				std::cout << "Added laser action to program actions\n";
				break;
			}

			default:
				break;
		}
	}
}

double Program::get_double(std::string::iterator &s)
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