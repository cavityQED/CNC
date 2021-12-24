#ifndef PROGRAM_H
#define PROGRAM_H

#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>

#include "action/laserAction.h"
#include "action/stepperAction.h"
#include "action/syncAction.h"

#include <QTimer>
#include <QTimerEvent>

namespace CNC
{

class Program : public QWidget
{
	Q_OBJECT
public:

	struct devicePointers
	{
		CNC::DEVICE::stepperMotor*	x_axis = nullptr;
		CNC::DEVICE::stepperMotor*	y_axis = nullptr;
		CNC::DEVICE::stepperMotor*	z_axis = nullptr;

		CNC::DEVICE::Laser*			laser = nullptr;
	};

	Program(QWidget* parent = nullptr);
	Program(const std::string filename, QWidget* parent = nullptr);
	~Program() {}

	void printBlocks() const;
	void setDevices(devicePointers devices)	{m_devices = devices;}

	virtual void timerEvent(QTimerEvent* e) override;

public slots:

	void start();
	void pause();
	void resume();
	void stop();
	void reset();
	void execute_next();

	void load();
	void load(const std::string& codeFileContents);
	void loadBlocks();	//Read text stored at m_filename and translate to codeBlocks
	void loadBlocks(const std::string& codeFileContents);
	void loadActions();	//Convert list of code blocks into list of program actions

public:

	const std::string	filename()	const {return m_filename;}
	const QString&		contents()	const {return m_fileContents;}
	size_t				blockSize()	const {return m_programBlocks.size();}

protected:
	//Helper functions for parsing code text file
	double	get_double(std::string::iterator &s); //Returns the first double found starting at position s
	double	get_double(std::string::const_iterator &s); //Returns the first double found starting at position s
	bool	is_supported_letter_code(const char c); //Returns true if c is a supported letter code (G, M, etc.)

protected:
	//Functions to get action params from code blocks

	CNC::Action*  G0_rapid						(CNC::codeBlock& block) {return new CNC::SyncAction(block);}
	CNC::Action*  G1_linearInterpolation		(const CNC::codeBlock& block) {return new CNC::SyncAction(block);}
	CNC::Action*  G2_circularInterpolationCW	(const CNC::codeBlock& block);
	CNC::Action*  G3_circularInterpolationCCW	(const CNC::codeBlock& block);

protected:

	std::fstream					m_codeFile;			//File stream for i/o on code text file
	std::string						m_filename;			//Location of the text code to run
	QString							m_fileContents;		//Contents of the code file - for use with Qt objects like QTextEdit
	std::vector<CNC::codeBlock>		m_programBlocks;	//List of individual code blocks in order of execution
	std::vector<CNC::Action*>		m_programActions;	//Sequence of actions executed when program is running
	bool							m_programMotion;	//True if the current program action is not yet completed
	devicePointers					m_devices;			//Pointers to the controllable devices used by the program

	int		m_timer;
	int 	m_programStep = 0;			//Current program step
	int		m_programTimerPeriod = 50;	//Period in ms of the program timer

};

}//CNC namespace

#endif