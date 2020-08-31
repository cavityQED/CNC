#include "JogController.h"
#include "PositionReadout.h"
#include "MainWindow.h"

#include <QApplication>

int main(int argc, char* argv[]) {
	QApplication a(argc, argv);
	MainWindow main;
	main.getPosition();
	main.show();

	return a.exec();
}
