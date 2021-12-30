#ifndef PROGRAM_H
#define PROGRAM_H

#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdlib.h>

#include <QTimer>
#include <QTimerEvent>

#include "common.h"
#include "device/stepperMotor.h"
#include "device/laser.h"

namespace CNC
{

class Program : public QWidget
{
	Q_OBJECT
public:

	Program(QWidget* parent = nullptr);
	Program(const std::string filename, QWidget* parent = nullptr);
	~Program() {}

	virtual void timerEvent(QTimerEvent* e) override;

public:

	void set_x_axis(CNC::DEVICE::stepperMotor*	x_axis)		{m_x_axis = x_axis;}
	void set_y_axis(CNC::DEVICE::stepperMotor*	y_axis)		{m_y_axis = y_axis;}
	void set_laser(CNC::DEVICE::Laser* laser)				{m_laser = laser;}

public slots:

	void start();
	void stop();
	void pause();
	void resume();
	void reset();
	void execute_next();

	void save();
	void load();
	void load(const std::string& codeFileContents);

public:

	size_t				size()		const {return m_programBlocks.size();	}
	size_t				counter()	const {return m_programStep;			}
	bool				eop()		const {return m_programStep == size();	}
	const std::string	filename()	const {return m_filename;				}
	const QString&		contents()	const {return m_fileContents;			}

	const std::vector<CNC::codeBlock*>& blocks() const {return m_programBlocks;} 

protected:
	//Helper functions for parsing code text file
	double	get_double(std::string::iterator &s); 		//Returns the first double found starting at position s
	double	get_double(std::string::const_iterator &s); //Overload
	bool	is_supported_letter_code(const char c); 	//Returns true if c is a supported letter code (G, M, etc.)

protected:

	bool							m_programMotion;	//True if the current program action is not yet completed
	QString							m_fileContents;		//Contents of the code file - for use with Qt objects like QTextEdit
	std::fstream					m_codeFile;			//File stream for i/o on code text file
	std::string						m_filename;			//Location of the text code to run
	std::vector<CNC::codeBlock*>	m_programBlocks;	//List of individual code blocks in order of execution

	int		m_timer;
	int		m_programTimerPeriod = 50;	//Period in ms of the program timer
	size_t 	m_programStep = 0;			//Current program step

	CNC::DEVICE::stepperMotor*		m_x_axis;
	CNC::DEVICE::stepperMotor*		m_y_axis;
	CNC::DEVICE::Laser*				m_laser;

};

}//CNC namespace

#endif