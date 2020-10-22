#include "JogController.h"

JogController::JogController(QWidget *parent) : QWidget(parent) {
	createButtons();
	setupBoxes();
	setShortcuts();
	connectButtons();
	setStyleSheets();
	
	QAction *enJogCont = new QAction;
	enJogCont->setShortcut(Qt::Key_F12);
	connect(enJogCont, &QAction::triggered, this, &JogController::enableJogContinuous);
	addAction(enJogCont);
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
	jog_min = new QPushButton(".01");
	jog_min->setCheckable(true);
	jog_low = new QPushButton(".1");
	jog_low->setCheckable(true);
	jog_med = new QPushButton("1");
	jog_med->setCheckable(true);
	jog_high = new QPushButton("2.5");
	jog_high->setCheckable(true);
}

void JogController::setupBoxes() {
	
	//Setup the Jog Button Box
	QGridLayout *axis_control_box_layout 	= new QGridLayout;
	axis_control_box			 			= new QGroupBox("Axis Control");
	axis_control_box_layout->addWidget(x_pos, 2, 3);
	axis_control_box_layout->addWidget(x_neg, 2, 1);
	axis_control_box_layout->addWidget(y_pos, 1, 2);
	axis_control_box_layout->addWidget(y_neg, 3, 2);
	axis_control_box_layout->addWidget(z_pos, 1, 3);
	axis_control_box_layout->addWidget(z_neg, 3, 1);
	axis_control_box_layout->setSpacing(5);
	axis_control_box_layout->setColumnStretch(0, 1);
	axis_control_box_layout->setColumnStretch(4, 1);
	axis_control_box_layout->setRowStretch(0, 1);
	axis_control_box_layout->setRowStretch(4, 1);
	axis_control_box->setFlat(true);
	axis_control_box->setLayout(axis_control_box_layout);
	
	//Setup the Jog Speed Box
	QVBoxLayout *jog_speed_layout	= new QVBoxLayout;
	jog_speed_box			 		= new QGroupBox("Jog Speed");
	jog_speed_layout->addWidget(jog_high);
	jog_speed_layout->addWidget(jog_med);
	jog_speed_layout->addWidget(jog_low);
	jog_speed_layout->addWidget(jog_min);
	jog_speed_box->setFlat(true);
	jog_speed_box->setLayout(jog_speed_layout);
	
	//Setup the Main Box
	QHBoxLayout *jog_mode_box_layout	= new QHBoxLayout;
	jog_mode_box 						= new QGroupBox("Jog Mode");
	jog_mode_box_layout->addWidget(axis_control_box);
	jog_mode_box_layout->addWidget(jog_speed_box);
	jog_mode_box->setFlat(true);
	jog_mode_box->setLayout(jog_mode_box_layout);	
	jog_mode_box->setCheckable(true);
	jog_mode_box->setChecked(false);
	jog_mode_box->setObjectName("mainBox");
	
	QVBoxLayout *jogController_layout = new QVBoxLayout;
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
		
	jog_high->setShortcut(Qt::Key_F2);
	jog_med->setShortcut(Qt::Key_F3);
	jog_low->setShortcut(Qt::Key_F4);
}

void JogController::connectButtons() {
	connect(x_pos, SIGNAL(pressed()), this, SLOT(xJogPos()));
	connect(x_neg, SIGNAL(pressed()), this, SLOT(xJogNeg()));
	connect(y_pos, SIGNAL(pressed()), this, SLOT(yJogPos()));
	connect(y_neg, SIGNAL(pressed()), this, SLOT(yJogNeg()));
	connect(z_pos, SIGNAL(pressed()), this, SLOT(zJogPos()));
	connect(z_neg, SIGNAL(pressed()), this, SLOT(zJogNeg()));
	
	connect(x_pos, SIGNAL(released()), this, SLOT(stop()));
	connect(x_neg, &QPushButton::released, this, &JogController::stop);
	connect(y_pos, &QPushButton::released, this, &JogController::stop);
	connect(y_neg, &QPushButton::released, this, &JogController::stop);
	
	connect(jog_mode_box, SIGNAL(toggled(bool)), this, SLOT(enableJogMode(bool)));
	
	connect(jog_min, SIGNAL(clicked(bool)), this, SLOT(setJogSpeedMin(bool)));
	connect(jog_low, SIGNAL(clicked(bool)), this, SLOT(setJogSpeedLow(bool)));
	connect(jog_med, SIGNAL(clicked(bool)), this, SLOT(setJogSpeedMed(bool)));
	connect(jog_high, SIGNAL(clicked(bool)), this, SLOT(setJogSpeedHigh(bool)));
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
						"max-width: 40px;"
						"min-height: 40px;"
						"max-height: 40px;}"
						
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
