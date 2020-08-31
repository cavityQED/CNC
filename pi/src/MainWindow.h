#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "JogController.h"
#include "PositionReadout.h"
#include "Curve.h"

#include <QAction>

#include <vector>
#include <cmath>
#include <iostream>

class MainWindow : public QWidget {
	Q_OBJECT
public:
	MainWindow(QWidget *parent = 0) : QWidget(parent){
		jog = new JogController;
		pos = new PositionReadout;
		controller = new MotorController(this);
		
		controller->setup_axis(xparams);
		controller->setup_axis(yparams);
		
		jog->setMotorController(controller);
				
		QHBoxLayout *layout = new QHBoxLayout;
		layout->addWidget(pos);
		layout->addWidget(jog);
		setLayout(layout);
				
		setStyleSheet("QWidget{background-color: #DDEFF2;}");
		
	}
	
	~MainWindow() {std::cout << "MainWindow destroyed\n";}
	
	void timerEvent(QTimerEvent *e) {
		double xpos;
		double ypos;
		
		controller->get_position(xpos, ypos);
		pos->setPosition(xpos, ypos);
		if(xpos == cur_xpos && ypos == cur_ypos) {
			killTimer(e->timerId());
		}
		else {
			cur_xpos = xpos;
			cur_ypos = ypos;
		}
	}
	
	void getPosition() {
		double x;
		double y;
		controller->get_position(x, y);
		pos->setPosition(x, y);
	}
		
			
private:
	JogController *jog;
	PositionReadout *pos;
	MotorController *controller;
	
	motor::params_t xparams = {25, 60, 400*200, 0, SPI::X_AXIS};
	motor::params_t yparams = {23, 60, 400*200, 0, SPI::Y_AXIS};
	
	double cur_xpos;
	double cur_ypos;
};

#endif
