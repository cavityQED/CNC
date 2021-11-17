#include "Program.h"

namespace CNC
{

Program::Program(const std::string filename, QWidget* parent) : QWidget(parent), m_filename(filename)
{

}

void Program::printBlocks()
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

void Program::run()
{
	while(runProgram)
	{

	}
}

void Program::pause()
{

}

void Program::resume()
{
	
}

void Program::stop()
{

}

void Program::loadBlocks()
{

	m_codeFile.open(m_filename);	//Open the code text file

	if(!m_codeFile.is_open())		//If file not open, there's nothing to do, return 
		return;						//TODO - Alert user

	//If file opened successfully, clear the program to load fresh
	m_programBlocks.clear();

	CNC::codeBlock			tmpBlock {};	//Temporary code block
	std::string				line;			//Line of text
	std::string::iterator	line_it;		//Text line iterator

	while(!m_codeFile.eof())	//Read the code text file until the end
	{
		//Get a line from the code file
		std::getline(m_codeFile, line);

		//line.push_back(' ');
		//std::cout << "LINE: " << line << "eof\n"; 

		//Reset the iterator to the beginning of the line
		line_it = line.begin();

		//Iterate through the line, creating codeBlocks as necessary
		while(line_it != line.end())
		{
			//Check if we're at the start of a supported letter code
			if(is_supported_letter_code(*line_it))
			{
				//If so, reset the temp block
				memset(&tmpBlock, 0, sizeof(CNC::codeBlock));

				//Set captial letter code
				tmpBlock.letterCode = (*line_it >= 97)? *line_it - 32 : *line_it;
				//Set the number code
				tmpBlock.numberCode = get_double(++line_it);

				//Populate codeBlock variables until another supported code letter or the end of the line is reached
				while(!(is_supported_letter_code(*line_it)) && line_it != line.end())
				{
					switch(*line_it)
					{
						case 'x': case 'X': tmpBlock.x = get_double(++line_it); break;
						case 'y': case 'Y': tmpBlock.y = get_double(++line_it); break;
						case 'z': case 'Z': tmpBlock.z = get_double(++line_it); break;

						case 'u': case 'U': tmpBlock.u = get_double(++line_it); break;
						case 'v': case 'V': tmpBlock.v = get_double(++line_it); break;
						case 'w': case 'W': tmpBlock.w = get_double(++line_it); break;

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
	}		
	//Close the file
	m_codeFile.close();	
}//loadBlocks

double Program::get_double(std::string::iterator &s)
{
	std::string d {""};	//Create empty string to store double

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