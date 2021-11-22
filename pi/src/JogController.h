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

#include "device/stepperMotor.h"

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

	void setXaxis(CNC::DEVICE::stepperMotor* x)	{x_axis = x;}
	void setYaxis(CNC::DEVICE::stepperMotor* y)	{y_axis = y;}
	void setZaxis(CNC::DEVICE::stepperMotor* z)	{z_axis = z;}
			
public slots:
	void enableJogMode(bool ena) {
		if(x_axis != nullptr)
			x_axis->esp_enable_jog_mode(ena);
		if(y_axis != nullptr)
			y_axis->esp_enable_jog_mode(ena);
		if(z_axis != nullptr)
			z_axis->esp_enable_jog_mode(ena);
	}
		
	void xJogPos() {
		if(x_axis != nullptr)
			x_axis->jogMove(1);
	}

	void xJogNeg() {
		if(x_axis != nullptr)
			x_axis->jogMove(0);
	}

	void yJogPos() {
		if(y_axis != nullptr)
			y_axis->jogMove(1);
	}
	void yJogNeg() {
		if(y_axis != nullptr)
			y_axis->jogMove(0);
	}
	void zJogPos() {}
	void zJogNeg() {}
	
	void setJogSpeedMin(bool checked) 
	{
		if(checked)
		{
			if(x_axis != nullptr)
				x_axis->setJogDistance(.01);
			if(y_axis != nullptr)
				y_axis->setJogDistance(.01);
			if(z_axis != nullptr)
				z_axis->setJogDistance(.01);
		
			jog_low->setChecked(false);
			jog_med->setChecked(false);
			jog_high->setChecked(false);
		}
		else
		{
			if(x_axis != nullptr)
				x_axis->setJogDistance(0);
			if(y_axis != nullptr)
				y_axis->setJogDistance(0);
			if(z_axis != nullptr)
				z_axis->setJogDistance(0);			
		}
	}

	void setJogSpeedLow(bool checked) 
	{
		if(checked)
		{
			if(x_axis != nullptr)
				x_axis->setJogDistance(.1);
			if(y_axis != nullptr)
				y_axis->setJogDistance(.1);
			if(z_axis != nullptr)
				z_axis->setJogDistance(.1);
		
			jog_min->setChecked(false);
			jog_med->setChecked(false);
			jog_high->setChecked(false);
		}
		else
		{
			if(x_axis != nullptr)
				x_axis->setJogDistance(0);
			if(y_axis != nullptr)
				y_axis->setJogDistance(0);
			if(z_axis != nullptr)
				z_axis->setJogDistance(0);			
		}
	}

	void setJogSpeedMed(bool checked) 
	{
		if(checked)
		{
			if(x_axis != nullptr)
				x_axis->setJogDistance(1);
			if(y_axis != nullptr)
				y_axis->setJogDistance(1);
			if(z_axis != nullptr)
				z_axis->setJogDistance(1);
		
			jog_min->setChecked(false);
			jog_low->setChecked(false);
			jog_high->setChecked(false);
		}
		else
		{
			if(x_axis != nullptr)
				x_axis->setJogDistance(0);
			if(y_axis != nullptr)
				y_axis->setJogDistance(0);
			if(z_axis != nullptr)
				z_axis->setJogDistance(0);			
		}
	}

	void setJogSpeedHigh(bool checked) 
	{
		if(checked)
		{
			if(x_axis != nullptr)
				x_axis->setJogDistance(2.5);
			if(y_axis != nullptr)
				y_axis->setJogDistance(2.5);
			if(z_axis != nullptr)
				z_axis->setJogDistance(2.5);
		
			jog_min->setChecked(false);
			jog_low->setChecked(false);
			jog_med->setChecked(false);
		}
		else
		{
			if(x_axis != nullptr)
				x_axis->setJogDistance(0);
			if(y_axis != nullptr)
				y_axis->setJogDistance(0);
			if(z_axis != nullptr)
				z_axis->setJogDistance(0);			
		}
	}
		
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

	CNC::DEVICE::stepperMotor* x_axis = nullptr;
	CNC::DEVICE::stepperMotor* y_axis = nullptr;
	CNC::DEVICE::stepperMotor* z_axis = nullptr;
	
	//Jog Speed Group Box
	QGroupBox *jog_speed_box;
	
	//Action to Toggle Jog Mode
	QAction *jog_toggle;
	
	double xPos;
	double yPos;
};

#endif
