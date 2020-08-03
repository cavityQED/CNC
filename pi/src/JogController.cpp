#include "JogController.h"

JogController::JogController(QWidget *parent) : QWidget(parent) {
	createButtons();
	setupBoxes();
	setShortcuts();
	connectButtons();
	setStyleSheets();
}

void JogController::createButtons() {
	//Create the Jog Buttons
	x_pos = new QPushButton("X+");
	x_neg = new QPushButton("X-");
	y_pos = new QPushButton("Y+");
	y_neg = new QPushButton("Y-");
	z_pos = new QPushButton("Z+");
	z_neg = new QPushButton("Z-");
	
	//Create the Jog Speed Buttons
	jog_p01mm = new QPushButton(".01");
	jog_p01mm->setCheckable(true);
	jog_p01mm->setAutoExclusive(true);
	jog_p1mm = new QPushButton(".1");
	jog_p1mm->setCheckable(true);
	jog_p1mm->setAutoExclusive(true);
	jog_1mm = new QPushButton("1");
	jog_1mm->setCheckable(true);
	jog_1mm->setAutoExclusive(true);
}

void JogController::setupBoxes() {
	
	//Setup the Jog Button Box
	axis_control_box 			= new QGroupBox("Axis Control");
	axis_control_box_layout 	= new QGridLayout;
	axis_control_box_layout->addWidget(x_pos, 1, 2);
	axis_control_box_layout->addWidget(x_neg, 1, 0);
	axis_control_box_layout->addWidget(y_pos, 0, 1);
	axis_control_box_layout->addWidget(y_neg, 2, 1);
	axis_control_box_layout->addWidget(z_pos, 0, 2);
	axis_control_box_layout->addWidget(z_neg, 2, 0);
	axis_control_box->setLayout(axis_control_box_layout);
	
	//Setup the Jog Speed Box
	jog_speed_box 		= new QGroupBox("Jog Speed (mm/step)");
	jog_speed_layout	= new QHBoxLayout;
	jog_speed_layout->addWidget(jog_1mm);
	jog_speed_layout->addWidget(jog_p1mm);
	jog_speed_layout->addWidget(jog_p01mm);
	jog_speed_box->setLayout(jog_speed_layout);
	
	//Setup the Main Box
	jog_mode_box 		= new QGroupBox("Jog Mode");
	jog_mode_box_layout	= new QVBoxLayout;
	jog_mode_box_layout->addWidget(axis_control_box);
	jog_mode_box_layout->addWidget(jog_speed_box);
	jog_mode_box->setLayout(jog_mode_box_layout);
	
	jog_mode_box->setCheckable(true);
	jog_mode_box->setChecked(false);
	jog_mode_box->setObjectName("mainBox");
	
	jogController_layout = new QVBoxLayout;
	jogController_layout->addWidget(jog_mode_box);
	
	setLayout(jogController_layout);
}

void JogController::setShortcuts() {
	x_pos->setShortcut(Qt::Key_6);
	x_neg->setShortcut(Qt::Key_4);
	y_pos->setShortcut(Qt::Key_8);
	y_neg->setShortcut(Qt::Key_2);
	z_pos->setShortcut(Qt::Key_9);
	z_neg->setShortcut(Qt::Key_1);
		
	jog_1mm->setShortcut(Qt::Key_F2);
	jog_p1mm->setShortcut(Qt::Key_F3);
	jog_p01mm->setShortcut(Qt::Key_F4);
	
	jog_toggle = new QAction(this);
	jog_toggle->setShortcut(Qt::Key_F1);
	connect(jog_toggle, SIGNAL(triggered(bool)), this, SLOT(enableJogMode(bool)));
	addAction(jog_toggle);
	
}

void JogController::connectButtons() {
	connect(x_pos, SIGNAL(pressed()), this, SLOT(xJogPos()));
	connect(x_neg, SIGNAL(pressed()), this, SLOT(xJogNeg()));
	connect(y_pos, SIGNAL(pressed()), this, SLOT(yJogPos()));
	connect(y_neg, SIGNAL(pressed()), this, SLOT(yJogNeg()));
	connect(z_pos, SIGNAL(pressed()), this, SLOT(zJogPos()));
	connect(z_neg, SIGNAL(pressed()), this, SLOT(zJogNeg()));
	
	connect(jog_mode_box, SIGNAL(clicked(bool)), this, SLOT(en_jog_mode(bool)));
	
	connect(jog_p01mm, SIGNAL(clicked(bool)), this, SLOT(setJogSpeedp01mm(bool)));
	connect(jog_p1mm, SIGNAL(clicked(bool)), this, SLOT(setJogSpeedp1mm(bool)));
	connect(jog_1mm, SIGNAL(clicked(bool)), this, SLOT(setJogSpeed1mm(bool)));
}

void JogController::setStyleSheets() {
	setStyleSheet(	"QPushButton{"	
						"background-color: #75B8C8;"
						"border-style: outset;"
						"border-width: 3px;"
						"border-color: #408DA0;"
						"border-radius: 4px;"
						"font: bold 18px;"
						"outline: 0;"
						"min-width: 40px;"
						"min-height: 40px;}"
						
					"QPushButton:pressed{"
						"background-color: #408DA0;"
						"border-style: inset;}"
						
					"QPushButton:checked{"
						"background-color: #0EFF5E;"
						"border-color: #049434;}"
												
					"QGroupBox#mainBox{"
						"border-style: inset;"
						"border: 3px solid gray;"
						"margin-top: 10px;"
						"padding-left: 5px;"
						"padding-top: 5px;"
						"outline: 0;"
						"font: bold 16px;}"
						
					"QGroupBox#mainBox::title{"
						"subcontrol-position: top left;"
						"subcontrol-origin: margin;"
						"top: 0; left: 8px;}"
						
					"QGroupBox{"
						"font: bold 14px;}");
}
