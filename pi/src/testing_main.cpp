#include "JogController.h"
#include "PositionReadout.h"
#include "MainWindow.h"
#include "Laser.h"
#include "Program.h"

#include <QApplication>

int main(int argc, char* argv[]) {
	QApplication a(argc, argv);
	sem_unlink("sem_ready");

	CNC::Program p("cnc.nc");
	p.loadBlocks();
	p.printBlocks();

	MainWindow main;
	main.show();

//	Laser laser;
//	laser.show();
	return a.exec();
}
