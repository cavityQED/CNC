#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "PositionReadout.h"
#include "program/program.h"

#include "device/laser.h"
#include "device/stepperMotor.h"
#include "control/knob.h"
#include "control/controlPanel.h"

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QMainWindow>

#include <vector>
#include <cmath>
#include <iostream>
#include <signal.h>

#define ESP_MOVED_SIGNAL 25

class MainWindow : public QMainWindow {
	Q_OBJECT
public:

	static void shutdown(int signum)
	{
		gpioTerminate();
		exit(signum);
	}

	MainWindow(QWidget *parent = 0) : QMainWindow(parent){

		pos = new PositionReadout;
		laser = new CNC::DEVICE::Laser;
		knob = new Knob(17, 27, this);
		panel = new CNC::ControlPanel;

		QVBoxLayout *vLayout = new QVBoxLayout;
		vLayout->addWidget(pos);
		vLayout->addWidget(laser);
		QHBoxLayout *layout = new QHBoxLayout;
		layout->addLayout(vLayout);
		layout->addWidget(panel);		
	
		QWidget *central = new QWidget;
		central->setLayout(layout);
		setCentralWidget(central);

		setStyleSheet("QWidget{background-color: #DDEFF2;}");	

		gpioInitialise();
		gpioSetSignalFunc(SIGINT, shutdown);
		x_axis = new CNC::DEVICE::stepperMotor(xparams);
		y_axis = new CNC::DEVICE::stepperMotor(yparams);

		connect(x_axis,			&CNC::DEVICE::stepperMotor::positionChange, 	pos,	&PositionReadout::setX);
		connect(y_axis,			&CNC::DEVICE::stepperMotor::positionChange, 	pos,	&PositionReadout::setY);

		connect(pos->x_zero(),	&QPushButton::released,				x_axis,		&CNC::DEVICE::stepperMotor::setHome);
		connect(pos->y_zero(),	&QPushButton::released,				y_axis,		&CNC::DEVICE::stepperMotor::setHome);

		connect(knob, 	&Knob::turn,	 					this,	&MainWindow::knobTurn);
		connect(panel,	&CNC::ControlPanel::modeChange,		this,	&MainWindow::setMode);
		connect(panel,	&CNC::ControlPanel::axisButton,		this,	&MainWindow::axisEvent);
		connect(panel,	&CNC::ControlPanel::jogSpeed,		this,	&MainWindow::setJogSpeed);
		connect(panel,	&CNC::ControlPanel::run, 			this,	&MainWindow::run);
		connect(panel,	&CNC::ControlPanel::pause,			this,	&MainWindow::pause);
		connect(panel,	&CNC::ControlPanel::reset,			this,	&MainWindow::reset);

		x_axis->esp_receive();
		y_axis->esp_receive();

		mdi_program = new CNC::Program();
		program = new CNC::Program("cnc.nc", this);
		program->load();

		for(auto b : program->blocks())
			std::cout << b << '\n';

		panel->textBox()->setPlainText(program->contents());

		QAction* run = new QAction;
		run->setShortcut(Qt::Key_F5);
		connect(run, &QAction::triggered, program, &CNC::Program::start);
		addAction(run);

		QAction* zero = new QAction;
		zero->setShortcut(Qt::Key_H);
		connect(zero, &QAction::triggered, x_axis, &CNC::DEVICE::stepperMotor::esp_find_zero);
		addAction(zero);

		QAction* pause = new QAction;
		pause->setShortcut(Qt::Key_P);
		connect(pause, &QAction::triggered, program, &CNC::Program::pause);
		addAction(pause);

		QAction* resume = new QAction;
		resume->setShortcut(Qt::Key_R);
		connect(resume, &QAction::triggered, program, &CNC::Program::resume);
		addAction(resume);

		QAction* load = new QAction;
		load->setShortcut(Qt::CTRL + Qt::Key_L);
		connect(load, &QAction::triggered, this, &MainWindow::loadProgramFromTextBox);
		addAction(load);

		setStyleSheet(	"QPushButton{"	
							"background-color: #75B8C8;"
							"border-style: outset;"
							"border-width: 3px;"
							"border-color: #408DA0;"
							"border-radius: 4px;"
							"font: bold 12pt;"
							"outline: 0;"
							"min-width: 40px;"
							"max-width: 40px;"
							"min-height: 40px;"
							"max-height: 40px;}"
						
						"QPushButton:pressed{"
							"background-color: #408DA0;"
							"border-style: inset;}"
						
						"QPushButton:checked{"
							"background-color: #0EFF5E;"
							"border-color: #049434;}");
	}
	
	~MainWindow() {std::cout << "MainWindow destroyed\n";}
	
	static void esp_moved_isr() {}
			
public slots:

	void knobTurn(bool cw)
	{
		if(m_mode == CNC::MODE::JOG)
			x_axis->jogMove(cw);
	}

	void setMode(CNC::MODE m)
	{
		switch(m)
		{
			case CNC::MODE::HOME:
				std::cout << "HOME mode set\n";
				break;

			case CNC::MODE::AUTO:
			{
				panel->textBox()->setText(program->contents());
				panel->textBox()->setReadOnly(true);
				std::cout << "AUTO mode set\n";
				break;
			}

			case CNC::MODE::JOG:
			{
				x_axis->jogEnable(true);
				y_axis->jogEnable(true);
				std::cout << "JOG mode set\n";
				break;
			}

			case CNC::MODE::EDIT:
				std::cout << "EDIT mode set\n";
				break;

			case CNC::MODE::MDI:
			{
				panel->textBox()->setText(mdi_program->contents());
				panel->textBox()->setReadOnly(false);
				std::cout << "MDI mode set\n";
				break;
			}
		}
		m_mode = m;
		reset();
	}

	void axisEvent(CNC::AXIS a, bool dir)
	{
		switch(a)
		{
			case CNC::AXIS::X:
			{
				if(m_mode == CNC::MODE::JOG)
					x_axis->jogMove(dir);
			
				else if(m_mode == CNC::MODE::HOME)
					x_axis->esp_find_zero();
			
				break;
			}

			case CNC::AXIS::Y:
			{	
				if(m_mode == CNC::MODE::JOG)
					y_axis->jogMove(dir);
			
				else if(m_mode == CNC::MODE::HOME)
					y_axis->esp_find_zero();
			
				break;
			}
		}
	}

	void setJogSpeed(int speed)
	{
		switch(speed)
		{
			case 100:
				x_axis->setJogDistance(2.5);
				y_axis->setJogDistance(2.5);
				break;

			case 10:
				x_axis->setJogDistance(1);
				y_axis->setJogDistance(1);
				break;

			case 1:
				x_axis->setJogDistance(0.1);
				y_axis->setJogDistance(0.1);
				break;
		}
	}

	void run()
	{
		switch(m_mode)
		{
			case CNC::MODE::AUTO:
			{
				if(m_reset)
				{
					program->start();
					m_reset = false;
				}
	
				else
					program->resume();

				break;
			}

			case CNC::MODE::MDI:
			{
				if(m_reset || mdi_program->eop())
				{
					loadProgramFromTextBox();
					mdi_program->start();
					m_reset = false;
				}

				else
					mdi_program->resume();

				break;
			}
		}
	}

	void pause()
	{
		switch(m_mode)
		{
			case CNC::MODE::AUTO:
			{
				program->pause();
				break;
			}

			case CNC::MODE::MDI:
			{
				mdi_program->pause();
				break;
			}
		}		
	}

	void reset()
	{
		switch(m_mode)
		{
			case CNC::MODE::AUTO:
			{
				std::cout << "\nResetting auto mode\n";
				program->reset();
				std::cout << "Reset auto mode\n";
				m_reset = true;
				break;
			}

			case CNC::MODE::MDI:
			{
				mdi_program->reset();
				panel->textBox()->setText(mdi_program->contents());
				m_reset = true;
				break;
			}
		}
	}

	void loadProgramFromTextBox()
	{
		mdi_program->load(panel->textBox()->document()->toPlainText().toStdString());
	}

private:

	bool									m_reset = true;
	Knob*									knob;
	CNC::Program*							program;
	CNC::Program*							mdi_program;
	CNC::MODE								m_mode {CNC::MODE::NOP};
	PositionReadout* 						pos;
	MotorController* 						controller;
	CNC::ControlPanel* 						panel;
	CNC::DEVICE::Laser*						laser;
	CNC::DEVICE::stepperMotor*				x_axis;
	CNC::DEVICE::stepperMotor*				y_axis;
	CNC::DEVICE::stepperMotor::params_t 	xparams {13, 400, 175, CNC::DEVICE::ESP::AXIS::x_axis};
	CNC::DEVICE::stepperMotor::params_t 	yparams {19, 400, 300, CNC::DEVICE::ESP::AXIS::y_axis};
};

#endif
