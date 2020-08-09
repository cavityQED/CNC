#include "SyncMoveController.h"

SyncMoveController::SyncMoveController(QWidget *parent) : QWidget(parent) {
	createWidgets();
	setStyleSheets();
	connectSlots();
}

void SyncMoveController::createWidgets() {
	//Create the axis-specific widgets
	x_move = new QWidget;
	y_move = new QWidget;
	z_move = new QWidget;
	
	//Create the labels
	QLabel *x_label = new QLabel("X:");
	QLabel *y_label = new QLabel("Y:");
	QLabel *z_label = new QLabel("Z:");
	
	//Create the line edits
	x_lineEdit = new QLineEdit;
	y_lineEdit = new QLineEdit;
	z_lineEdit = new QLineEdit;
	
	x_lineEdit->setAlignment(Qt::AlignRight);
	y_lineEdit->setAlignment(Qt::AlignRight);
	z_lineEdit->setAlignment(Qt::AlignRight);
	
	//Create the set steps buttons
	set_x_steps = new QPushButton("Set X Steps");
	set_y_steps = new QPushButton("Set Y Steps");
	set_z_steps = new QPushButton("Set Z Steps");
	
	
	//Create the axis widget layouts
	QHBoxLayout *x_move_layout = new QHBoxLayout;
	QHBoxLayout *y_move_layout = new QHBoxLayout;
	QHBoxLayout *z_move_layout = new QHBoxLayout;
	
	//Add the widgets to the layouts
	x_move_layout->addWidget(x_label);
	x_move_layout->addWidget(x_lineEdit);
	x_move_layout->addWidget(set_x_steps);
	
	y_move_layout->addWidget(y_label);
	y_move_layout->addWidget(y_lineEdit);
	y_move_layout->addWidget(set_y_steps);
	
	z_move_layout->addWidget(z_label);
	z_move_layout->addWidget(z_lineEdit);
	z_move_layout->addWidget(set_z_steps);
	
	//Set the widget layouts
	x_move->setLayout(x_move_layout);
	y_move->setLayout(y_move_layout);
	z_move->setLayout(z_move_layout);
	
	//Create the sync move button
	sync_move_button = new QPushButton("Sync Move");
	
	//Create the circle move button
	circle_move_button = new QPushButton("Circle Move");
	
	//Put the move buttons in a widget
	QWidget *buttons = new QWidget;
	QHBoxLayout *but_layout = new QHBoxLayout;
	but_layout->addWidget(sync_move_button);
	but_layout->addWidget(circle_move_button);
	buttons->setLayout(but_layout);
	
	//Create and setup the main group box
	sync_mode_box = new QGroupBox("Sync Mode");
	sync_mode_box->setCheckable(true);
	sync_mode_box->setChecked(false);
	QVBoxLayout *sync_box_layout = new QVBoxLayout;
	sync_box_layout->addWidget(x_move);
	sync_box_layout->addWidget(y_move);
	sync_box_layout->addWidget(z_move);
	sync_box_layout->addWidget(buttons);
	sync_mode_box->setLayout(sync_box_layout);
	
	//Setup the main layout and add the main box
	QVBoxLayout *main_layout = new QVBoxLayout;
	main_layout->addWidget(sync_mode_box);
	setLayout(main_layout);
}

void SyncMoveController::setStyleSheets() {
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
						"top: 0; left: 8px;}"
						
					"QPushButton{"	
						"background-color: #75B8C8;"
						"border-style: outset;"
						"border-width: 3px;"
						"border-color: #408DA0;"
						"border-radius: 4px;"
						"font: bold 18px;"
						"outline: 0;"
						"min-height: 40px;}"
						
					"QPushButton:pressed{"
						"background-color: #408DA0;"
						"border-style: inset;}");
}

void SyncMoveController::connectSlots() {
	connect(set_x_steps, SIGNAL(pressed()), this, SLOT(send_x_steps()));
	connect(set_y_steps, SIGNAL(pressed()), this, SLOT(send_y_steps()));
	connect(set_z_steps, SIGNAL(pressed()), this, SLOT(send_z_steps()));
	
	connect(sync_move_button, SIGNAL(pressed()), this, SLOT(sync_move()));
	connect(circle_move_button, SIGNAL(pressed()), this, SLOT(circle_move()));
	
	connect(sync_mode_box, SIGNAL(clicked(bool)), this, SLOT(enable_sync_mode(bool)));
}
