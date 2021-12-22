#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "JogController.h"
#include "PositionReadout.h"
#include "Program.h"

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

		connect(knob, 			&Knob::turn,	 						this, 		&MainWindow::knobTurn);
		connect(panel,			&CNC::ControlPanel::modeChange,			this,		&MainWindow::setMode);
		connect(panel,			&CNC::ControlPanel::axisButton,			this,		&MainWindow::axisEvent);
		connect(panel,			&CNC::ControlPanel::jogSpeed,			this,		&MainWindow::setJogSpeed);

		x_axis->esp_receive();
		y_axis->esp_receive();

		CNC::Program::devicePointers devices;
		devices.x_axis = x_axis;
		devices.y_axis = y_axis;
		devices.laser = laser;

		program = new CNC::Program("cnc.nc", this);
		program->setDevices(devices);
		program->loadBlocks();
		program->printBlocks();
		program->loadActions();
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
							"font: bold 18px;"
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
				std::cout << "AUTO mode set\n";
				break;
			case CNC::MODE::JOG:
				std::cout << "JOG mode set\n";
				x_axis->jogEnable(true);
				y_axis->jogEnable(true);
				break;
			case CNC::MODE::EDIT:
				std::cout << "EDIT mode set\n";
				break;
			case CNC::MODE::MDI:
				std::cout << "MDI mode set\n";
				break;
		}
		m_mode = m;
	}

	void axisEvent(CNC::AXIS a, bool dir)
	{
		switch(a)
		{
			case CNC::AXIS::X:
				if(m_mode == CNC::MODE::JOG)
					x_axis->jogMove(dir);
				else if(m_mode == CNC::MODE::HOME)
					x_axis->esp_find_zero();
				break;
			case CNC::AXIS::Y:
				if(m_mode == CNC::MODE::JOG)
					y_axis->jogMove(dir);
				else if(m_mode == CNC::MODE::HOME)
					y_axis->esp_find_zero();
				break;
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

	void loadProgramFromTextBox()
	{
		program->loadBlocks(panel->textBox()->document()->toPlainText().toStdString());
		program->printBlocks();
		program->loadActions();
	}

private:
	PositionReadout *pos;
	MotorController *controller;
	Knob* knob;

	CNC::ControlPanel* panel;

	CNC::DEVICE::stepperMotor::params_t xparams {13, 200, 200, CNC::DEVICE::ESP::AXIS::x_axis};
	CNC::DEVICE::stepperMotor::params_t yparams {19, 200, 200, CNC::DEVICE::ESP::AXIS::y_axis};
	
	CNC::DEVICE::stepperMotor*	x_axis;
	CNC::DEVICE::stepperMotor*	y_axis;
	CNC::DEVICE::Laser*			laser;

	CNC::Program* program;

	CNC::MODE	m_mode = CNC::MODE::NOP;
};

#endif
