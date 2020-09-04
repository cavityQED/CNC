#ifndef POSITIONREADOUT_H
#define POSITIONREADOUT_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QTimer>
#include <QTimerEvent>

#include <iostream>

#include "MotorController.h"

class PositionReadout : public QWidget {
	Q_OBJECT
public:
	PositionReadout(QWidget *parent = 0);
	~PositionReadout() {std::cout << "PositionReadout destroyed\n";}
	
	void createWidgets();
	void setStyleSheets();
			
	void setPosition(double xpos, double ypos, double zpos = 0) {
		x_lineEdit->setText(QString::number(xpos, 'f', 2));
		y_lineEdit->setText(QString::number(ypos, 'f', 2));
		z_lineEdit->setText(QString::number(zpos, 'f', 2));
	}
	
public slots:
	void updatePosition(double x, double y) {
		setPosition(x, y);
	}
	
private:	
	//Main Group Box
	QGroupBox *axis_position_box;
	
	//Main Group Box Layout
	QVBoxLayout *axis_position_box_layout;
	
	//Axis-Specific Position Widgets	
	QWidget *x_pos;
	QWidget *y_pos;
	QWidget *z_pos;
	
	//Axis-Specific Line Edits
	//Positions will be displayed here
	QLineEdit *x_lineEdit;
	QLineEdit *y_lineEdit;
	QLineEdit *z_lineEdit;
	
	//Axis-Specific Layouts
	QHBoxLayout *x_pos_layout;
	QHBoxLayout *y_pos_layout;
	QHBoxLayout *z_pos_layout;	
};

#endif
