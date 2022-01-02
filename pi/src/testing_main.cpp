#include "MainWindow.h"

#include <QApplication>

int main(int argc, char* argv[]) {
	QApplication a(argc, argv);
	sem_unlink("spi_ready_semaphore");

	MainWindow main;
	main.show();

	return a.exec();
}
