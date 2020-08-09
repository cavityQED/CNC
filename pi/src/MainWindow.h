#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "JogController.h"
#include "PositionReadout.h"
#include "SyncMoveController.h"
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
		sync = new SyncMoveController;
		controller = new MotorController(this);
		
		controller->setup_x_axis(xparams);
		controller->setup_y_axis(yparams);
		
		jog->setMotorController(controller);
		sync->setMotorController(controller);
				
		QHBoxLayout *layout = new QHBoxLayout;
		layout->addWidget(pos);
		layout->addWidget(jog);
		layout->addWidget(sync);
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
			controller->set_motion(false);
		}
		else {
			cur_xpos = xpos;
			cur_ypos = ypos;
		}
	}
			
private:
	JogController *jog;
	PositionReadout *pos;
	SyncMoveController *sync;
	MotorController *controller;
	
	motorParameters xparams = {25, 800, .005};
	motorParameters yparams = {23, 800, .005};
	
	double cur_xpos;
	double cur_ypos;
};

#endif
