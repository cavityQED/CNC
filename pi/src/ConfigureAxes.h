#ifndef CONFIGUREAXES_H
#define CONFIGUREAXES_H

#include <vector>

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QString>

#include "Types.h"
#include "utilities/ConfigureUtility.h"

class ConfigureAxes : public QWidget {
	Q_OBJECT
public:
	ConfigureAxes(QWidget *parent = 0);
	~ConfigureAxes() {}
		
	void load_initial_from_file();
	
public slots:
	void updateAxis();
	void saveExit();
	
	void showInfo(int index);
	
signals:
	void axisConfigChange();
	
private:
	QLineEdit *gpio;
	QLineEdit *spmm;
	QLineEdit *max_travel;
	QLineEdit *backlash;
	
	QComboBox *axis_select;
	
	QPushButton *update_axis;
	QPushButton *save_exit;
	
	motor::params_t xparams;
	motor::params_t yparams;
	motor::params_t zparams = {0};
};

#endif
