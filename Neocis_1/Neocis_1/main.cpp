#include "Neocis_1.h"
#include <QtWidgets/QApplication>

// Standard Qt main
int main(int argc, char *argv[]) {
	QApplication application(argc, argv);

	Neocis_1 window;
	window.show();

	return application.exec();
}
