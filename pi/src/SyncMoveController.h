#ifndef SYNCMOVECONTROLLER_H
#define SYNCMOVECONTROLLER_H

#include "MotorController.h"

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QLabel>

class SyncMoveController : public QWidget {
	Q_OBJECT
public:
	SyncMoveController(QWidget *parent = 0);
	~SyncMoveController() {std::cout << "SyncController destroyed\n";}
	
	void createWidgets();
	void setStyleSheets();
	void connectSlots();
	
	void setMotorController(MotorController *controller) {
		motorController = controller;
	}
	
public slots:
	void enable_sync_mode(bool enable) {
		motorController->enable_sync_mode(enable);
		motorController->enable_step_mode(enable);
	}
	
	void send_x_steps() {
		motorController->x_set_steps_to_move(x_lineEdit->text().toInt());
	}
	
	void send_y_steps() {
		motorController->y_set_steps_to_move(y_lineEdit->text().toInt());
	}
	
	void send_z_steps() {}
	
	void sync_move() {
		motorController->sync_move();
		parent()->startTimer(25);
	}
	
	void circle_move() {
		motorController->circle_move();
		parent()->startTimer(50);
	}
	
private:
	//Main Group Box
	QGroupBox *sync_mode_box;
	
	//Axis-Specific Widgets
	QWidget	*x_move;
	QWidget	*y_move;
	QWidget	*z_move;
	
	//Line Edits for Entering Step Counts
	QLineEdit *x_lineEdit;
	QLineEdit *y_lineEdit;
	QLineEdit *z_lineEdit;
	
	//Push Buttons to Send Steps to Axis
	QPushButton *set_x_steps;
	QPushButton *set_y_steps;
	QPushButton *set_z_steps;
	
	//Button to Execute Move
	QPushButton *sync_move_button;
	
	//Circle Move Button
	QPushButton *circle_move_button;
	
	//Motor Controller
	MotorController *motorController;
};

#endif
	
	
