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
#include "device/stepperGroup.h"
#include "device/laser.h"

#define MOTION_PIN	17

namespace CNC
{

class Program : public QWidget
{
	Q_OBJECT

public:

	static void triggerISR(int gpio, int level, uint32_t tick, void* arg)
	{
		if(!level)
		{
			std::cout << "\n*****Triggering Next Move*****\n";
			((CNC::Program*)arg)->triggered();
		}
	}

	Program(QWidget* parent = nullptr);
	Program(const std::string filename, QWidget* parent = nullptr);
	~Program() {sem_unlink("program_semaphore");}

	virtual void timerEvent(/*QTimerEvent* e*/);// override;

public:

	void set_axes(CNC::DEVICE::StepperGroup* group)		{m_axes = group;}
	void set_laser(CNC::DEVICE::Laser* laser)			{m_laser = laser;}
	void set_motion(bool motion)						{m_programMotion = motion;}

signals:

	void stepperBlock(const CNC::codeBlock* b);
	void laserBlock(const CNC::codeBlock* b);
	void triggered();

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

	bool							m_programMotion;			//True if the current program action is not yet completed
	int								m_programTimerPeriod = 10;	//Period in ms of the program timer
	size_t 							m_programStep = 0;			//Current program step
	QTimer*							m_timer;

	QString							m_fileContents;		//Contents of the code file - for use with Qt objects like QTextEdit
	std::fstream					m_codeFile;			//File stream for i/o on code text file
	std::string						m_filename;			//Location of the text code to run
	std::vector<CNC::codeBlock*>	m_programBlocks;	//List of individual code blocks in order of execution

	CNC::DEVICE::StepperGroup*		m_axes;
	CNC::DEVICE::Laser*				m_laser;
};

}//CNC namespace

#endif