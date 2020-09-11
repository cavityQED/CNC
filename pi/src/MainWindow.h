#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "JogController.h"
#include "PositionReadout.h"
#include "Curve.h"

#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QMainWindow>

#include <vector>
#include <cmath>
#include <iostream>

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	MainWindow(QWidget *parent = 0) : QMainWindow(parent){
		jog = new JogController;
		pos = new PositionReadout;
		controller = new MotorController(this);
		
		connect(controller, &MotorController::positionChanged, pos, &PositionReadout::setPosition);
		connect(jog, &JogController::jog, controller, &MotorController::jog);
		connect(jog, &JogController::setJog, controller, &MotorController::setJog);
		connect(jog, &JogController::enableJog, controller, &MotorController::enableJog);
		
		controller->setup_axis(xparams);
		controller->setup_axis(yparams);
		controller->set_jog_steps(0);
		
		double x, y;
		controller->get_position(x, y);
		pos->setPosition(x, y);
						
		QHBoxLayout *layout = new QHBoxLayout;
		layout->addWidget(pos);
		layout->addWidget(jog);
		
		QWidget *central = new QWidget;
		central->setLayout(layout);
		setCentralWidget(central);
				
		popup = new QWidget;
		popup->setFixedSize(300,300);
		
		QMenu *config = menuBar()->addMenu("Configure");
		QAction *axes = new QAction("Axes");
		config->addAction(axes);
		connect(axes, &QAction::triggered, popup, &QWidget::show);
		
		setStyleSheet("QWidget{background-color: #DDEFF2;}");
	}
	
	~MainWindow() {std::cout << "MainWindow destroyed\n";}
			
public slots:
	
private:
	JogController *jog;
	PositionReadout *pos;
	MotorController *controller;
	
	motor::params_t xparams = {25, 60, 400*200, 0, SPI::X_AXIS};
	motor::params_t yparams = {23, 60, 400*200, 0, SPI::Y_AXIS};
	
	double cur_xpos;
	double cur_ypos;
	
	QWidget *popup;
};

#endif
