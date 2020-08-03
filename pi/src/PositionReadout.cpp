#include "PositionReadout.h"

PositionReadout::PositionReadout(QWidget *parent) : QWidget(parent) {
	createWidgets();
	setStyleSheets();
}

void PositionReadout::createWidgets() {
	//X Axis
	QLabel	*x_label	= new QLabel("X:");
	QLabel	*x_unit		= new QLabel("mm");
	x_lineEdit			= new QLineEdit;
	x_pos				= new QWidget;
	x_pos_layout		= new QHBoxLayout;
	x_pos_layout->addWidget(x_label);
	x_pos_layout->addWidget(x_lineEdit);
	x_pos_layout->addWidget(x_unit);
	x_pos_layout->setAlignment(Qt::AlignVCenter);
	x_pos->setLayout(x_pos_layout);
	x_lineEdit->setAlignment(Qt::AlignRight);
	x_lineEdit->setReadOnly(true);
	x_lineEdit->setText(QString::number(0, 'f', 2));
	
	//Y Axis
	QLabel	*y_label	= new QLabel("Y:");
	QLabel	*y_unit		= new QLabel("mm");
	y_lineEdit			= new QLineEdit;
	y_pos				= new QWidget;
	y_pos_layout		= new QHBoxLayout;
	y_pos_layout->addWidget(y_label);
	y_pos_layout->addWidget(y_lineEdit);
	y_pos_layout->addWidget(y_unit);
	y_pos_layout->setAlignment(Qt::AlignVCenter);
	y_pos->setLayout(y_pos_layout);
	y_lineEdit->setAlignment(Qt::AlignRight);
	y_lineEdit->setReadOnly(true);
	y_lineEdit->setText(QString::number(0, 'f', 2));
	
	//Z Axis
	QLabel	*z_label	= new QLabel("Z:");
	QLabel	*z_unit		= new QLabel("mm");
	z_lineEdit			= new QLineEdit;
	z_pos				= new QWidget;
	z_pos_layout		= new QHBoxLayout;
	z_pos_layout->addWidget(z_label);
	z_pos_layout->addWidget(z_lineEdit);
	z_pos_layout->addWidget(z_unit);
	z_pos_layout->setAlignment(Qt::AlignVCenter);
	z_pos->setLayout(z_pos_layout);
	z_lineEdit->setAlignment(Qt::AlignRight);
	z_lineEdit->setReadOnly(true);
	z_lineEdit->setText(QString::number(0, 'f', 2));

	//Main Box and Layout
	axis_position_box			= new QGroupBox("Axis Positions");
	axis_position_box_layout	= new QVBoxLayout;
	axis_position_box_layout->addWidget(x_pos);
	axis_position_box_layout->addWidget(y_pos);
	axis_position_box_layout->addWidget(z_pos);
//	axis_position_box_layout->setSpacing(0);
//	axis_position_box_layout->setContentsMargins(0,0,0,0);
//	axis_position_box_layout->setSizeConstraint(QLayout::SetMaximumSize);
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
						"font: DSEG 22px;"
						"color: white;}"
						
					"QLabel{"
						"font: bold 20px;}"
						
					"QGroupBox{"
						"border-style: inset;"
						"border: 3px solid gray;"
						"margin-top: 10px;"
						"padding-left: 5px;"
						"padding-top: 5px;"
						"outline: 0;"
						"font: bold 16px;}"
						
					"QGroupBox::title{"
						"subcontrol-position: top left;"
						"subcontrol-origin: margin;"
						"top: 0; left: 8px;}");
}
	
	
