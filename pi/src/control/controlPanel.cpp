#include "controlPanel.h"

namespace CNC
{

ControlPanel::ControlPanel(QWidget* parent) : QWidget(parent)
{
	setupModeGroup();
	setupArrowGroup();
	setupJogGroup();
	setupControlGroup();
	setupTextBox();

	QVBoxLayout* vbox = new QVBoxLayout();
	vbox->addWidget(m_mode_group);
	vbox->addWidget(m_arrow_group);

	QHBoxLayout* hbox = new QHBoxLayout();
	hbox->addWidget(m_control_group);
	hbox->addLayout(vbox);
	hbox->addWidget(m_jog_group);

	QVBoxLayout* main = new QVBoxLayout();
	main->addWidget(m_textBox);
	main->addLayout(hbox);

	setLayout(main);

	setStyleSheet(	"QPushButton{"	
						"background-color: #75B8C8;"
						"border-style: outset;"
						"border-width: 3px;"
						"border-color: #408DA0;"
						"border-radius: 4px;"
						"font: bold 16pt;"
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

					"QGroupBox{"
						"border-style: inset;"
						"border: 3px solid gray;"
						"margin-top: 10px;"
						"padding-left: 5px;"
						"padding-top: 5px;"
						"outline: 0;"
						"font: bold 8pt;}"
						
					"QGroupBox::title{"
						"subcontrol-position: top left;"
						"subcontrol-origin: margin;"
						"top: 0; left: 8px;}");
}

void ControlPanel::setupModeGroup()
{
	//Create the mode buttons
	m_jog_button	= new QPushButton("JOG");
	m_home_button	= new QPushButton("HOME");
	m_auto_button	= new QPushButton("AUTO");
	m_MDI_button	= new QPushButton("MDI");
	m_edit_button	= new QPushButton("EDIT");
	m_two_button	= new QPushButton("2");

	//Connect mode buttons to modeChange signal
	connect(m_jog_button,	&QPushButton::clicked, [this] {modeChange(CNC::MODE::JOG);	});
	connect(m_home_button,	&QPushButton::clicked, [this] {modeChange(CNC::MODE::HOME);	});
	connect(m_auto_button,	&QPushButton::clicked, [this] {modeChange(CNC::MODE::AUTO);	});
	connect(m_MDI_button,	&QPushButton::clicked, [this] {modeChange(CNC::MODE::MDI);	});
	connect(m_edit_button,	&QPushButton::clicked, [this] {modeChange(CNC::MODE::EDIT);	});

	//Make buttons checkable
	m_jog_button->setCheckable(true);
	m_home_button->setCheckable(true);
	m_auto_button->setCheckable(true);
	m_MDI_button->setCheckable(true);
	m_edit_button->setCheckable(true);
	m_two_button->setCheckable(true);

	//Add the buttons to an exclusive button group so only one can be checked at a time
	QButtonGroup* mode_group = new QButtonGroup();
	mode_group->setExclusive(true);
	mode_group->addButton(m_jog_button);
	mode_group->addButton(m_home_button);
	mode_group->addButton(m_auto_button);
	mode_group->addButton(m_MDI_button);
	mode_group->addButton(m_edit_button);
	mode_group->addButton(m_two_button);

	//Add the buttons to a grid layout
	QGridLayout* mode_layout = new QGridLayout();
	mode_layout->addWidget(m_jog_button, 0, 0);
	mode_layout->addWidget(m_home_button, 0, 1);
	mode_layout->addWidget(m_auto_button, 0, 2);
	mode_layout->addWidget(m_MDI_button, 1, 0);
	mode_layout->addWidget(m_edit_button, 1, 1);
	mode_layout->addWidget(m_two_button, 1, 2);

	//Create the mode group box and set the layout
	m_mode_group = new QGroupBox();
	m_mode_group->setLayout(mode_layout);

	m_mode_group->setStyleSheet("QPushButton{"	
									"background-color: #75B8C8;"
									"border-style: outset;"
									"border-width: 3px;"
									"border-color: #408DA0;"
									"border-radius: 4px;"
									"font: bold 10pt;"
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
									"border-color: #049434;}");
}

void ControlPanel::setupArrowGroup()
{
	//Create the arrow buttons
	m_left_button	= new QPushButton(QChar(0x2190));
	m_up_button		= new QPushButton(QChar(0x2191));
	m_right_button	= new QPushButton(QChar(0x2192));
	m_down_button	= new QPushButton(QChar(0x2193));

	//Add the buttons to a grid layout
	QGridLayout* arrow_layout = new QGridLayout();
	arrow_layout->addWidget(m_up_button, 0, 1);
	arrow_layout->addWidget(m_left_button, 1, 0);
	arrow_layout->addWidget(m_down_button, 1, 1);
	arrow_layout->addWidget(m_right_button, 1, 2);

	//Create the arrow group and set the layout
	m_arrow_group = new QGroupBox();
	m_arrow_group->setLayout(arrow_layout);
}

void ControlPanel::setupJogGroup()
{
	//Create the jog buttons
	m_jog100_button		= new QPushButton("100");
	m_jog10_button		= new QPushButton("10");
	m_jog1_button		= new QPushButton("1");

	connect(m_jog100_button,	&QPushButton::clicked, [this] {jogSpeed(100);	});
	connect(m_jog10_button,		&QPushButton::clicked, [this] {jogSpeed(10);	});
	connect(m_jog1_button,		&QPushButton::clicked, [this] {jogSpeed(1);		});

	m_jogXpos_button	= new QPushButton("X+");
	m_jogXneg_button	= new QPushButton("X-");
	m_jogYpos_button	= new QPushButton("Y+");
	m_jogYneg_button	= new QPushButton("Y-");
	m_jogZpos_button	= new QPushButton("Z+");
	m_jogZneg_button	= new QPushButton("Z-");

	m_jogXpos_button->setShortcut(Qt::Key_6);
	m_jogXneg_button->setShortcut(Qt::Key_4);
	m_jogYpos_button->setShortcut(Qt::Key_8);
	m_jogYneg_button->setShortcut(Qt::Key_2);

	connect(m_jogXpos_button,	&QPushButton::clicked, [this] {axisButton(CNC::AXIS::X, true);	});
	connect(m_jogXneg_button,	&QPushButton::clicked, [this] {axisButton(CNC::AXIS::X, false);	});
	connect(m_jogYpos_button,	&QPushButton::clicked, [this] {axisButton(CNC::AXIS::Y, true);	});
	connect(m_jogYneg_button,	&QPushButton::clicked, [this] {axisButton(CNC::AXIS::Y, false);	});

	//Set the speed buttons as checkable and add to an exclusive group
	m_jog100_button->setCheckable(true);
	m_jog10_button->setCheckable(true);
	m_jog1_button->setCheckable(true);

	QButtonGroup* speed = new QButtonGroup();
	speed->addButton(m_jog100_button);
	speed->addButton(m_jog10_button);
	speed->addButton(m_jog1_button);
	speed->setExclusive(true);

	//Add the buttons to a grid layout
	QGridLayout* jog_layout = new QGridLayout();
	jog_layout->addWidget(m_jog100_button, 0, 0);
	jog_layout->addWidget(m_jog10_button, 0, 1);
	jog_layout->addWidget(m_jog1_button, 0, 2);

	jog_layout->addWidget(m_jogXpos_button, 2, 2);
	jog_layout->addWidget(m_jogXneg_button, 2, 0);

	jog_layout->addWidget(m_jogYpos_button, 1, 1);
	jog_layout->addWidget(m_jogYneg_button, 3, 1);

	jog_layout->addWidget(m_jogZpos_button, 1, 2);
	jog_layout->addWidget(m_jogZneg_button, 3, 0);

	//Create the jog group and set the layout
	m_jog_group = new QGroupBox();
	m_jog_group->setLayout(jog_layout);
}

void ControlPanel::setupControlGroup()
{
	m_run_button	= new QPushButton("RUN");
	m_hold_button	= new QPushButton("HOLD");
	m_reset_button	= new QPushButton("RESET");

	connect(m_run_button,	&QPushButton::clicked, this, &ControlPanel::run);
	connect(m_hold_button,	&QPushButton::clicked, this, &ControlPanel::pause);
	connect(m_reset_button,	&QPushButton::clicked, this, &ControlPanel::reset);

	QGridLayout* control_layout = new QGridLayout();
	control_layout->addWidget(m_reset_button, 0, 0);
	control_layout->addWidget(m_run_button, 3, 0);
	control_layout->addWidget(m_hold_button, 3, 1);

	m_control_group = new QGroupBox();
	m_control_group->setLayout(control_layout);

	m_control_group->setStyleSheet("QPushButton{"	
										"background-color: #75B8C8;"
										"border-style: outset;"
										"border-width: 3px;"
										"border-color: #408DA0;"
										"border-radius: 4px;"
										"font: bold 10pt;"
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
										"border-color: #049434;}");
}

void ControlPanel::setupTextBox()
{
	m_textBox = new QTextEdit();
	m_textBox->setFontPointSize(16);
	m_textBox->setStyleSheet("font-size: 16pt");
}

}//CNC namespace