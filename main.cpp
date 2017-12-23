#include <QApplication>
#include <QMessageBox>

#include "Passwords.h"
#include "Dialog.h"

static int run(QApplication&);
static std::string get_db_path();

int main(int argc, char **argv){
	QApplication app(argc, argv);

	return run(app);
}

int run(QApplication &app){
	try{
		// init the db
		Manager mgr(get_db_path());

		// ask user for master password
		Greeter greeter;
		if(!greeter.exec())
			return 1;
		const std::string master = greeter.password();

		// open the db
		mgr.open(master);

		Passwords passwords(mgr);
	}catch(const Manager::NotFound&){
		if(!Manager::generate(get_db_path())){
			QMessageBox::critical(NULL, "Error", ("Could not generate the database at \"" + get_db_path() + "\"!").c_str());
			return 1;
		}

		// recurse
		return run(app);
	}catch(const Manager::Corrupt&){
		QMessageBox::critical(NULL, "Error", ("The Passwords database at \"" + get_db_path() + "\" appears to be corrupt.").c_str());
		return 2;
	}

	return app.exec();
}

#ifndef _WIN32
#include <wordexp.h>
std::string get_db_path(){
	wordexp_t p;

	wordexp("~/.passwordsdb", &p, 0);
	const std::string path = p.we_wordv[0];
	wordfree(&p);

	return path;
}
#endif // _WIN32
