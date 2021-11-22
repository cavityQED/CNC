#include "JogController.h"
#include "PositionReadout.h"
#include "MainWindow.h"
#include "Laser.h"
#include "Program.h"

#include <QApplication>

int main(int argc, char* argv[]) {
	QApplication a(argc, argv);
	sem_unlink("spi_ready_semaphore");

/*
	gpioInitialise();
	CNC::DEVICE::stepperMotor::params_t x_params {5, 200, 200, true};
	CNC::DEVICE::stepperMotor::params_t y_params {6, 200, 200, false};

	CNC::DEVICE::stepperMotor* x_axis = new CNC::DEVICE::stepperMotor(x_params);
	CNC::DEVICE::stepperMotor* y_axis = new CNC::DEVICE::stepperMotor(y_params);

	CNC::DEVICE::Laser*			laser = new CNC::DEVICE::Laser();

	CNC::Program::devicePointers devices;
	devices.x_axis = x_axis;
	devices.y_axis = y_axis;
	devices.laser = laser;

	CNC::Program p("cnc.nc");
	p.setDevices(devices);
	p.loadBlocks();
	p.printBlocks();
	p.loadActions();

	p.start();
*/
	MainWindow main;
	main.show();

	return a.exec();
}
