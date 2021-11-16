#ifndef PROGRAM_H
#define PROGRAM_H

#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>

#include "Action.h"

namespace CNC
{

class Program : public QWidget
{
	Q_OBJECT
public:

	Program(const std::string filename, QWidget* parent = nullptr);
	~Program() {}

	void printBlocks();

public slots:

	void run();
	void pause();
	void resume();
	void stop();

	void loadBlocks();	//Read text stored at m_filename and translate to codeBlocks

public:

	const std::string	filename()	{return m_filename;}
	size_t				blockSize()	{return m_programBlocks.size();}
	bool				isRunning()	{return runProgram;}

protected:
	//Helper functions for parsing code text file
	double	get_double(std::string::iterator &s); //Returns the first double found starting at position s
	bool	is_supported_letter_code(const char c); //Returns true if c is a supported letter code (G, M, etc.)

protected:

	std::fstream					m_codeFile;			//File stream for i/o on code text file
	std::string						m_filename;			//Location of the text code to run
	std::vector<CNC::codeBlock>		m_programBlocks;	//List of individual code blocks in order of execution
	std::vector<CNC::Action>		m_programActions;	//Sequence of actions executed when program is running

	bool	runProgram = false;
};

}//CNC namespace

#endif