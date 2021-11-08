#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "JogController.h"
#include "PositionReadout.h"
#include "Curve.h"
#include "Laser.h"
#include "ConfigureAxes.h"
#include "utilities/ConfigureUtility.h"

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
		jog = new JogController;
		pos = new PositionReadout;
		laser = new Laser;
		controller = new MotorController(this);
		
		connect(controller, &MotorController::positionChanged, pos, &PositionReadout::setPosition);
		connect(jog, &JogController::jog_event, controller, &MotorController::jog_event_handler);
		
		ConfigureUtility configure;
		configure.get_axis_params(SPI::X_AXIS, xparams);
		configure.get_axis_params(SPI::Y_AXIS, yparams);
		
		controller->setup_axis(xparams);
		controller->setup_axis(yparams);
		controller->set_jog_steps(0);
		
		double x, y;
		controller->get_position(x, y);
		pos->setPosition(x, y);
						
		QVBoxLayout *vLayout = new QVBoxLayout;
		vLayout->addWidget(pos);
		vLayout->addWidget(laser);
		QHBoxLayout *layout = new QHBoxLayout;
		layout->addLayout(vLayout);
		layout->addWidget(jog);
		
		QWidget *central = new QWidget;
		central->setLayout(layout);
		setCentralWidget(central);
				
		popup = new ConfigureAxes(this);
		popup->setWindowFlags(Qt::Window);
		popup->setWindowModality(Qt::WindowModal);
		popup->setWindowTitle("Configure Axes");
		connect(popup, &ConfigureAxes::axisConfigChange, controller, &MotorController::updateAxisConfig);
		
		QMenu *config = menuBar()->addMenu("Configure");
		QAction *axes = new QAction("Axes");
		config->addAction(axes);
		connect(axes, &QAction::triggered, this, &MainWindow::displayConfigureWindow);
		
		setStyleSheet("QWidget{background-color: #DDEFF2;}");	
		
		wiringPiISR(ESP_MOVED_SIGNAL, INT_EDGE_RISING, esp_moved_isr);
		
		
	}
	
	~MainWindow() {std::cout << "MainWindow destroyed\n";}
	
	static void esp_moved_isr() {}
			
public slots:
	void displayConfigureWindow() {
		popup->show();
		popup->move(this->geometry().center() - popup->rect().center());
	}
	
private:
	JogController *jog;
	PositionReadout *pos;
	MotorController *controller;
	Laser* laser;
	
	motor::params_t xparams {};
	motor::params_t yparams {};
	
	double cur_xpos;
	double cur_ypos;
	
	ConfigureAxes *popup;
};

#endif
