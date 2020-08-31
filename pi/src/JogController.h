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
	
	void setMotorController(MotorController *controller) {
		motorController = controller;
	}
		
public slots:
	void en_jog_mode(bool ena) {
		motorController->enable_jog_mode(ena);
	}
	
	void xJogPos() {
		motorController->set_dir(SPI::X_AXIS, 1);
		motorController->move(SPI::X_AXIS);
		parent()->startTimer(50);
	}
	void xJogNeg() {
		motorController->set_dir(SPI::X_AXIS, 0);
		motorController->move(SPI::X_AXIS);
		parent()->startTimer(50);
	}
	void yJogPos() {
		motorController->set_dir(SPI::Y_AXIS, 1);
		motorController->move(SPI::Y_AXIS);
		parent()->startTimer(50);
	}
	void yJogNeg() {
		motorController->set_dir(SPI::Y_AXIS, 0);
		motorController->move(SPI::Y_AXIS);
		parent()->startTimer(50);
	}
	void zJogPos() {}
	void zJogNeg() {}
	
	void setJogSpeedp01mm(bool checked) {
		if(checked)
			motorController->set_jog_speed_mm(.01);
	}
	void setJogSpeedp1mm(bool checked) {
		if(checked)
			motorController->set_jog_speed_mm(0.1);
	}
	void setJogSpeed1mm(bool checked) {
		if(checked)
			motorController->set_jog_speed_mm(1);
	}
	
	//Connect action to jog mode box so it can be un/checked with keystroke
	void enableJogMode(bool b) {
		jog_mode_box->setChecked(!jog_mode_box->isChecked());
		jog_mode_box->emit clicked(jog_mode_box->isChecked());
	}
	
private:
	//Jog Controller Layout
	QVBoxLayout *jogController_layout;
	
	//Main Group Box
	//Will be a checkable box to activate/deactivate jog mode
	QGroupBox *jog_mode_box;
	
	//Main Group Box Layout
	QVBoxLayout *jog_mode_box_layout;
	
	//Jog Buttons
	QPushButton *x_pos;
	QPushButton *x_neg;
	QPushButton *y_pos;
	QPushButton *y_neg;
	QPushButton *z_pos;
	QPushButton *z_neg;
	
	//Jog Button Group Box
	QGroupBox *axis_control_box;
	
	//Jog Button Layout
	QGridLayout *axis_control_box_layout;
	
	//Jog Speed Buttons
	QPushButton *jog_p01mm;
	QPushButton *jog_p1mm;
	QPushButton *jog_1mm;
	
	//Jog Speed Group Box
	QGroupBox *jog_speed_box;
	
	//Jog Speed Button Layout
	QHBoxLayout *jog_speed_layout;
	
	//Action to Toggle Jog Mode
	QAction *jog_toggle;
	
	//MotorController
	MotorController *motorController;
	
};

#endif
