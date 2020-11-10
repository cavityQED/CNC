#ifndef JOGCONTROLLER_H
#define JOGCONTROLLER_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QTimer>
#include <QTimerEvent>
#include <QAction>

#include "MotorController.h"
#include "Curve.h"

class JogController : public QWidget {
	Q_OBJECT
public:
	JogController(QWidget *parent = 0);
	~JogController() {std::cout << "JogController destroyed\n";}
	
	void createButtons();
	void setupBoxes();
	void setShortcuts();
	void connectButtons();
	void setStyleSheets();
			
public slots:
	void enableJogMode(bool ena) {
		event.type = JOG::ENABLE_JOG;
		event.enable = ena;
		emit jog_event(event);
	}
		
	void xJogPos() {
		event.type = JOG::JOG_MOVE;
		event.axis = SPI::X_AXIS;
		event.direction = 1;
		emit jog_event(event);
	}
	void xJogNeg() {
		event.type = JOG::JOG_MOVE;
		event.axis = SPI::X_AXIS;
		event.direction = 0;
		emit jog_event(event);
	}
	void yJogPos() {
		event.type = JOG::JOG_MOVE;
		event.axis = SPI::Y_AXIS;
		event.direction = 1;
		emit jog_event(event);
	}
	void yJogNeg() {
		event.type = JOG::JOG_MOVE;
		event.axis = SPI::Y_AXIS;
		event.direction = 0;
		emit jog_event(event);
	}
	void zJogPos() {}
	void zJogNeg() {}
	
	void setJogSpeedMin(bool checked) {
		event.type = JOG::SET_JOG_SPEED_MM;
		if(checked) {
			event.jog_mm = 0.01;
			jog_low->setChecked(false);
			jog_med->setChecked(false);
			jog_high->setChecked(false);
		}
		else
			event.jog_mm = 0;
			
		emit jog_event(event);
	}
	void setJogSpeedLow(bool checked) {
		event.type = JOG::SET_JOG_SPEED_MM;
		if(checked) {
			event.jog_mm = 0.1;
			jog_min->setChecked(false);
			jog_med->setChecked(false);
			jog_high->setChecked(false);
		}
		else
			event.jog_mm = 0;
			
		emit jog_event(event);
	}
	void setJogSpeedMed(bool checked) {
		event.type = JOG::SET_JOG_SPEED_MM;
		if(checked) {
			event.jog_mm = 1;
			jog_min->setChecked(false);
			jog_low->setChecked(false);
			jog_high->setChecked(false);
		}
		else
			event.jog_mm = 0;
			
		emit jog_event(event);
	}
	void setJogSpeedHigh(bool checked) {
		event.type = JOG::SET_JOG_SPEED_MM;
		if(checked) {
			event.jog_mm = 2.5;
			jog_min->setChecked(false);
			jog_low->setChecked(false);
			jog_med->setChecked(false);
		}
		else
			event.jog_mm = 0;
			
		emit jog_event(event);
	}
		
signals:	
	void jog_event(JOG::event_t &event);
	
private:
	//Main Group Box
	//Will be a checkable box to activate/deactivate jog mode
	QGroupBox *jog_mode_box;
	
	//Jog Buttons
	QPushButton *x_pos;
	QPushButton *x_neg;
	QPushButton *y_pos;
	QPushButton *y_neg;
	QPushButton *z_pos;
	QPushButton *z_neg;
	
	//Jog Button Group Box
	QGroupBox *axis_control_box;
	
	//Jog Speed Buttons
	QPushButton *jog_min;
	QPushButton *jog_low;
	QPushButton *jog_med;
	QPushButton *jog_high;
	
	//Jog Speed Group Box
	QGroupBox *jog_speed_box;
	
	//Action to Toggle Jog Mode
	QAction *jog_toggle;
	
	//MotorController
	MotorController *motorController;
	
	double xPos;
	double yPos;
		
	JOG::event_t event;
};

#endif
