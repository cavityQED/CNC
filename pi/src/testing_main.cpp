#include "JogController.h"
#include "PositionReadout.h"
#include "MainWindow.h"
#include "Laser.h"
#include "Program.h"

#include "control/modeSelect.h"

#include <QApplication>

int main(int argc, char* argv[]) {
	QApplication a(argc, argv);
	sem_unlink("spi_ready_semaphore");

	MainWindow main;
	main.show();

	return a.exec();
}
