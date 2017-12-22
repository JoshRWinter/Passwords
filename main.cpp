#include <QApplication>

#include "Passwords.h"

int main(int argc, char **argv){
	QApplication app(argc, argv);

	Passwords passwords;
	passwords.show();

	return app.exec();
}
