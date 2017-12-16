#include <QtWidgets/QApplication>

#include "harp_window.h"

int main( int argc, char* argv[])
{
	QApplication harp_simulator(argc, argv);

	HarpWindow w;

    return harp_simulator.exec();

}
