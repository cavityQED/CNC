#include "PositionReadout.h"

PositionReadout::PositionReadout(QWidget *parent) : QWidget(parent) {
	unit = "mm";
	createWidgets();
	setStyleSheets();
}

void PositionReadout::createWidgets() {
	//X Axis
	QLabel	*x_label	= new QLabel("X:");
	home_x				= new QPushButton("Zero");
	x_lineEdit			= new QLineEdit;
	x_pos				= new QWidget;
	x_pos_layout		= new QHBoxLayout;
	x_pos_layout->addWidget(home_x);
	x_pos_layout->addWidget(x_label);
	x_pos_layout->addWidget(x_lineEdit);
	x_pos_layout->setAlignment(Qt::AlignVCenter);
	x_pos->setLayout(x_pos_layout);
	x_lineEdit->setAlignment(Qt::AlignRight);
	x_lineEdit->setReadOnly(true);
	x_lineEdit->setText(QString::number(0, 'f', 2));
	
	//Y Axis
	QLabel	*y_label	= new QLabel("Y:");
	home_y				= new QPushButton("Zero");
	y_lineEdit			= new QLineEdit;
	y_pos				= new QWidget;
	y_pos_layout		= new QHBoxLayout;
	y_pos_layout->addWidget(home_y);
	y_pos_layout->addWidget(y_label);
	y_pos_layout->addWidget(y_lineEdit);
	y_pos_layout->setAlignment(Qt::AlignVCenter);
	y_pos->setLayout(y_pos_layout);
	y_lineEdit->setAlignment(Qt::AlignRight);
	y_lineEdit->setReadOnly(true);
	y_lineEdit->setText(QString::number(0, 'f', 2));
	
	//Z Axis
	QLabel	*z_label	= new QLabel("Z:");
	home_z				= new QPushButton("Zero");
	z_lineEdit			= new QLineEdit;
	z_pos				= new QWidget;
	z_pos_layout		= new QHBoxLayout;
	z_pos_layout->addWidget(home_z);
	z_pos_layout->addWidget(z_label);
	z_pos_layout->addWidget(z_lineEdit);
	z_pos_layout->setAlignment(Qt::AlignVCenter);
	z_pos->setLayout(z_pos_layout);
	z_lineEdit->setAlignment(Qt::AlignRight);
	z_lineEdit->setReadOnly(true);
	z_lineEdit->setText(QString::number(0, 'f', 2));

	//Main Box and Layout
	axis_position_box			= new QGroupBox("Position");
	axis_position_box_layout	= new QVBoxLayout;
	axis_position_box_layout->addWidget(x_pos);
	axis_position_box_layout->addWidget(y_pos);
	axis_position_box_layout->addWidget(z_pos);
	axis_position_box->setFlat(true);
	axis_position_box->setLayout(axis_position_box_layout);
	
	//Position Readout Layout
	QVBoxLayout *main_layout = new QVBoxLayout();
	main_layout->addWidget(axis_position_box);
	setLayout(main_layout);
	setFixedHeight(210);
}

void PositionReadout::setStyleSheets() {
	setStyleSheet(	"QLineEdit{"
						"min-width: 80px;"
						"min-height: 40px;"
						"background-color: #234D58;"
						"font: bold 20px;"
						"color: white;}"
						
					"QLabel{"
						"font: bold 25px;"
						"min-height: 40px;"
						"min-width: 30px;}"
						
					"QGroupBox{"
						"border-style: inset;"
						"border: 3px solid gray;"
						"margin-top: 10px;"
						"padding-left: 0px;"
						"padding-top: 5px;"
						"outline: 0;"
						"font: bold 16px;}"
						
					"QGroupBox::title{"
						"subcontrol-position: top left;"
						"subcontrol-origin: margin;"
						"top: 0; left: 8px;}");
}
	
void PositionReadout::setPosition(double x, double y, double z) {
	x_lineEdit->setText(QString::number(x, 'f', 2) + ' ' + unit);
	y_lineEdit->setText(QString::number(y, 'f', 2) + ' ' + unit);
	z_lineEdit->setText(QString::number(z, 'f', 2) + ' ' + unit);
}

void PositionReadout::setX(double x)
{
	x_lineEdit->setText(QString::number(x, 'f', 2) + ' ' + unit);
}

void PositionReadout::setY(double y)
{
	y_lineEdit->setText(QString::number(y, 'f', 2) + ' ' + unit);
}

void PositionReadout::setZ(double z)
{
	z_lineEdit->setText(QString::number(z, 'f', 2) + ' ' + unit);
}
