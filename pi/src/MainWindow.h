#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "JogController.h"
#include "PositionReadout.h"

class MainWindow : public QWidget {
	Q_OBJECT
public:
	MainWindow(QWidget *parent = 0) : QWidget(parent){
		jog = new JogController;
		pos = new PositionReadout;
		controller.setup_x_axis(xparams);
		controller.setup_y_axis(yparams);
		
		jog->setMotorController(&controller);
		pos->setMotorController(&controller);
		
		QHBoxLayout *layout = new QHBoxLayout;
		layout->addWidget(pos);
		layout->addWidget(jog);
		setLayout(layout);
		
		setStyleSheet("QWidget{background-color: #DDEFF2;}");
	}
	
	void timerEvent(QTimerEvent *e) {
		double xpos;
		double ypos;
		
		controller.get_position(xpos, ypos);
		pos->setPosition(xpos, ypos);
		if(xpos == cur_xpos && ypos == cur_ypos)
			killTimer(e->timerId());
		else {
			cur_xpos = xpos;
			cur_ypos = ypos;
		}
	}
		
private:
	JogController *jog;
	PositionReadout *pos;
	MotorController controller;
	
	motorParameters xparams = {6, 800, .005};
	motorParameters yparams = {4, 800, .005};
	
	double cur_xpos;
	double cur_ypos;
};

#endif
