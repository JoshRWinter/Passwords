#include <QApplication>
#include <QMessageBox>

#include "Passwords.h"
#include "Dialog.h"

static int run(QApplication&);
static std::string get_db_path();

#ifdef _WIN32
#include <windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int){
	int count = 0;
	QApplication app(count, NULL);

	return run(app);
}
#else
int main(int argc, char **argv){
	QApplication app(argc, argv);

	return run(app);
}
#endif // _WIN32

int run(QApplication &app){
	try{
		Manager mgr(get_db_path());

		// ask user for master password
		Greeter greeter;
		if(!greeter.exec())
			return 1;
		const std::string master = greeter.password();

		// open the db
		mgr.open(master);

		Passwords passwords(mgr);
		passwords.show();

		return app.exec();

	}catch(const Manager::NotFound&){
		// ask user for initial master password
		NewMaster newm;
		if(!newm.exec())
			return 1;
		const std::string master = newm.password();

		try{
			Manager::generate(get_db_path(), master);
		}catch(const Manager::ManagerException &e){
			QMessageBox::critical(NULL, "Error", e.what());
			return 1;
		}

		// recurse
		return run(app);
	}catch(const Manager::Corrupt&){
		QMessageBox::critical(NULL, "Error", ("The Passwords database at \"" + get_db_path() + "\" appears to be corrupt.").c_str());
		return 1;
	}catch(const Manager::IncorrectPassword&){
		QMessageBox::critical(NULL, "Error", "Could not unlock the database with that password!");

		// recurse
		return run(app);
	}
	catch(const std::exception &e){
		QMessageBox::critical(NULL, "error", e.what());
	}

	return 1;
}

#ifdef _WIN32
std::string get_db_path(){
	char path[MAX_PATH];
	ExpandEnvironmentStrings("%USERPROFILE%\\Documents\\PasswordsDB", path, MAX_PATH - 1);

	return path;
}
#else
#include <wordexp.h>
std::string get_db_path(){
	wordexp_t p;

	wordexp("~/.passwordsdb", &p, 0);
	const std::string path = p.we_wordv[0];
	wordfree(&p);

	return path;
}
#endif // _WIN32
