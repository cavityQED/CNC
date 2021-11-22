#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "JogController.h"
#include "PositionReadout.h"
#include "Program.h"

#include "device/laser.h"
#include "device/stepperMotor.h"

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QMainWindow>

#include <vector>
#include <cmath>
#include <iostream>

#define ESP_MOVED_SIGNAL 25

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow(QWidget *parent = 0) : QMainWindow(parent){

		pos = new PositionReadout;
		laser = new CNC::DEVICE::Laser;
		jog = new JogController;

		QVBoxLayout *vLayout = new QVBoxLayout;
		vLayout->addWidget(pos);
		vLayout->addWidget(laser);
		QHBoxLayout *layout = new QHBoxLayout;
		layout->addLayout(vLayout);
		layout->addWidget(jog);
		
		QWidget *central = new QWidget;
		central->setLayout(layout);
		setCentralWidget(central);

		setStyleSheet("QWidget{background-color: #DDEFF2;}");	

		gpioInitialise();
		x_axis = new CNC::DEVICE::stepperMotor(xparams);
		y_axis = new CNC::DEVICE::stepperMotor(yparams);

		jog->setXaxis(x_axis);
		jog->setYaxis(y_axis);

		CNC::Program::devicePointers devices;
		devices.x_axis = x_axis;
		devices.y_axis = y_axis;
		devices.laser = laser;

		connect(x_axis, &CNC::DEVICE::stepperMotor::positionChange, pos, &PositionReadout::setX);
		connect(y_axis, &CNC::DEVICE::stepperMotor::positionChange, pos, &PositionReadout::setY);

		program = new CNC::Program("cnc.nc", this);
		program->setDevices(devices);
		program->loadBlocks();
		program->printBlocks();
		program->loadActions();

		QAction* run = new QAction;
		run->setShortcut(Qt::Key_F5);
		connect(run, &QAction::triggered, program, &CNC::Program::start);
		addAction(run);
	}
	
	~MainWindow() {std::cout << "MainWindow destroyed\n";}
	
	static void esp_moved_isr() {}
			
public slots:
	
private:
	JogController *jog;
	PositionReadout *pos;
	MotorController *controller;
	
	CNC::DEVICE::stepperMotor::params_t xparams {5, 200, 200, true};
	CNC::DEVICE::stepperMotor::params_t yparams {6, 200, 200, false};
	
	CNC::DEVICE::stepperMotor*	x_axis;
	CNC::DEVICE::stepperMotor*	y_axis;
	CNC::DEVICE::Laser*			laser;

	CNC::Program* program;

	double cur_xpos;
	double cur_ypos;
	
};

#endif
