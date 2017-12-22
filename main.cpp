#include <QApplication>

#include "Passwords.h"
#include "Dialog.h"

int main(int argc, char **argv){
	QApplication app(argc, argv);

	Greeter greeter;
	if(!greeter.exec())
		return 1;

	const std::string passwd = greeter.password();

	Passwords passwords(passwd);
	passwords.show();

	return app.exec();
}
