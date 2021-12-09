#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "JogController.h"
#include "PositionReadout.h"
#include "Program.h"

#include "device/laser.h"
#include "device/stepperMotor.h"
#include "control/modeSelect.h"
#include "control/knob.h"

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
		jog = new JogController;

		CNC::CONTROL_WIDGET::ModeSelect* mode = new CNC::CONTROL_WIDGET::ModeSelect();

		QVBoxLayout *vLayout = new QVBoxLayout;
		vLayout->addWidget(pos);
		vLayout->addWidget(laser);
		vLayout->addWidget(mode);
		QHBoxLayout *layout = new QHBoxLayout;
		layout->addLayout(vLayout);
		layout->addWidget(jog);
		
		QWidget *central = new QWidget;
		central->setLayout(layout);
		setCentralWidget(central);

		setStyleSheet("QWidget{background-color: #DDEFF2;}");	

		gpioInitialise();
		gpioSetSignalFunc(SIGINT, shutdown);
		x_axis = new CNC::DEVICE::stepperMotor(xparams);
		y_axis = new CNC::DEVICE::stepperMotor(yparams);

		connect(x_axis, &CNC::DEVICE::stepperMotor::positionChange, pos, &PositionReadout::setX);
		connect(y_axis, &CNC::DEVICE::stepperMotor::positionChange, pos, &PositionReadout::setY);

		jog->setXaxis(x_axis);
		jog->setYaxis(y_axis);

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

		QAction* run = new QAction;
		run->setShortcut(Qt::Key_F5);
		connect(run, &QAction::triggered, program, &CNC::Program::start);
		addAction(run);

		QAction* zero = new QAction;
		zero->setShortcut(Qt::Key_H);
		connect(zero, &QAction::triggered, x_axis, &CNC::DEVICE::stepperMotor::esp_find_zero);
		addAction(zero);

		knob_setup();

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
	
private:
	JogController *jog;
	PositionReadout *pos;
	MotorController *controller;
	
	CNC::DEVICE::stepperMotor::params_t xparams {13, 200, 200, CNC::DEVICE::ESP::AXIS::x_axis};
	CNC::DEVICE::stepperMotor::params_t yparams {19, 200, 200, CNC::DEVICE::ESP::AXIS::y_axis};
	
	CNC::DEVICE::stepperMotor*	x_axis;
	CNC::DEVICE::stepperMotor*	y_axis;
	CNC::DEVICE::Laser*			laser;

	CNC::Program* program;

	double cur_xpos;
	double cur_ypos;
	
};

#endif
