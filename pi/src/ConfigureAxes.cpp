#include "ConfigureAxes.h"

ConfigureAxes::ConfigureAxes(QWidget *parent) : QWidget(parent) {
	QWidget *top = new QWidget;
	axis_select = new QComboBox;
	axis_select->addItem("X Axis");
	axis_select->addItem("Y Axis");
	axis_select->addItem("Z Axis");	
	QLabel *select = new QLabel("Axis: ");
	QHBoxLayout *top_layout = new QHBoxLayout;
	top_layout->addWidget(select);
	top_layout->addWidget(axis_select);
	top->setLayout(top_layout);
	
	QWidget *pin = new QWidget;
	QLabel *pin_label = new QLabel("GPIO Pin:\t");
	gpio = new QLineEdit;
	gpio->setAlignment(Qt::AlignRight);
	QHBoxLayout *pin_layout = new QHBoxLayout;
	pin_layout->addWidget(pin_label);
	pin_layout->addWidget(gpio);
	pin->setLayout(pin_layout);
	
	QWidget *steps = new QWidget;
	QLabel *steps_label = new QLabel("Steps/mm:\t");
	spmm = new QLineEdit;
	spmm->setAlignment(Qt::AlignRight);
	QHBoxLayout *steps_layout = new QHBoxLayout;
	steps_layout->addWidget(steps_label);
	steps_layout->addWidget(spmm);
	steps->setLayout(steps_layout);
	
	QWidget *max = new QWidget;
	QLabel *max_label = new QLabel("Max Travel (mm):\t");
	max_travel = new QLineEdit;
	max_travel->setAlignment(Qt::AlignRight);
	QHBoxLayout *max_layout = new QHBoxLayout;
	max_layout->addWidget(max_label);
	max_layout->addWidget(max_travel);
	max->setLayout(max_layout);
	
	QWidget *back = new QWidget;
	QLabel *back_label = new QLabel("Backlash:\t");
	backlash = new QLineEdit;
	backlash->setAlignment(Qt::AlignRight);
	QHBoxLayout *back_layout = new QHBoxLayout;
	back_layout->addWidget(back_label);
	back_layout->addWidget(backlash);
	back->setLayout(back_layout);
	
	QWidget *buttons = new QWidget;
	update_axis = new QPushButton("Update Axis");
	save_exit = new QPushButton("Save && Exit");
	QHBoxLayout *button_layout = new QHBoxLayout;
	button_layout->addWidget(update_axis);
	button_layout->addWidget(save_exit);
	buttons->setLayout(button_layout);
	
	QVBoxLayout *main_layout = new QVBoxLayout;
	main_layout->addWidget(top);
	main_layout->addWidget(pin);
	main_layout->addWidget(steps);
	main_layout->addWidget(max);
	main_layout->addWidget(back);
	main_layout->addWidget(buttons);
	setLayout(main_layout);
	setFixedSize(300, 350);
	setStyleSheet("QLineEdit {background-color: white;}"
					
					"QLabel{"
						"font: bold;}"
						
					"QPushButton{"	
						"background-color: #75B8C8;"
						"border-style: outset;"
						"border-width: 3px;"
						"border-color: #408DA0;"
						"border-radius: 4px;"
						"font: bold 18px;"
						"outline: 0;}"
						
					"QPushButton:pressed{"
						"background-color: #408DA0;"
						"border-style: inset;}"
						
					"QComboBox{"
						"font: bold;"
						"outline: 0;"
						"background: #75B8C8;}");
						
	connect(update_axis, &QPushButton::clicked, this, &ConfigureAxes::updateAxis);
	connect(save_exit, &QPushButton::clicked, this, &ConfigureAxes::saveExit);
	connect(axis_select, SIGNAL(activated(int)), this, SLOT(showInfo(int)));
	
	load_initial_from_file();
}

void ConfigureAxes::load_initial_from_file() {
	ConfigureUtility config;
	config.get_axis_params(SPI::X_AXIS, xparams);
	std::cout << "Got x params\n";
	config.get_axis_params(SPI::Y_AXIS, yparams);
	std::cout << "Got y params\n";
	config.get_axis_params(SPI::Z_AXIS, zparams);
	std::cout << "Got z params\n";
	
	showInfo(0);
}		

void ConfigureAxes::updateAxis() {
	QString a = axis_select->currentText();
	motor::params_t *p;
	
	if(a == "X Axis") {
		p = &xparams;
		p->a = SPI::X_AXIS;
	}
	else if (a == "Y Axis") {
		p = &yparams;
		p->a = SPI::Y_AXIS;
	}
	else if(a == "Z Axis") {
		p = &zparams;
		p->a = SPI::Z_AXIS;
	}
	
	p->pin_num = gpio->text().toInt();
	p->spmm = spmm->text().toInt();
	p->max_steps = max_travel->text().toDouble()*p->spmm;
	p->backlash = backlash->text().toDouble()*p->spmm;
}

void ConfigureAxes::saveExit() {
	ConfigureUtility config;
	if(xparams.pin_num)
		config.set_axis_params(xparams);
	if(yparams.pin_num)
		config.set_axis_params(yparams);
	if(zparams.pin_num)
		config.set_axis_params(zparams);
	emit axisConfigChange();
	hide();
}

void ConfigureAxes::showInfo(int index) {
	motor::params_t *p;
	switch(index) {
		case 0:
			p = &xparams;
			break;
		case 1:
			p = &yparams;
			break;
		case 2:
			p = &zparams;
			break;
		default:
			*p = {0};
	}
	
	gpio->setText(QString::number(p->pin_num));
	spmm->setText(QString::number(p->spmm));
	max_travel->setText(QString::number((double)p->max_steps/p->spmm));
	backlash->setText(QString::number((double)p->backlash/p->spmm));
}
