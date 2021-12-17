#ifndef JOGCONTROLLER_H
#define JOGCONTROLLER_H

#include <QWidget>
#include <QPushButton>
#include <QButtonGroup>
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
			
public slots:

	void xJogPos() 	{	emit jog_x(true);	}
	void xJogNeg() 	{	emit jog_x(false);	}
	void yJogPos() 	{	emit jog_y(true);	}
	void yJogNeg() 	{	emit jog_y(false);	}

	void zJogPos() {}
	void zJogNeg() {}
	
	void setJogSpeedMin(bool checked)	{	emit jog_set_distance(.01);	}
	void setJogSpeedLow(bool checked)	{	emit jog_set_distance(.1);	}
	void setJogSpeedMed(bool checked)	{	emit jog_set_distance(1);	} 
	void setJogSpeedMax(bool checked)	{	emit jog_set_distance(2.5);	}

signals:

	void jog_x				(bool dir);
	void jog_y				(bool dir);
	void jog_enable			(bool enable);
	void jog_set_distance	(double mm);

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
	QPushButton *jog_max;
	
	//Jog Speed Group Box
	QGroupBox *jog_speed_box;
	
	//Action to Toggle Jog Mode
	QAction *jog_toggle;
	
	double xPos;
	double yPos;

	bool	m_enabled = false;
};

#endif
