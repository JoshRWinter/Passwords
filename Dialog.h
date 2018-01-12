#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QLineEdit>

#include "Manager.h"
#include "Passwords.h"

// ask for master password
class Greeter:public QDialog{
public:
	Greeter();
	std::string password()const;
private:
	QLineEdit *pass;
};

// generate new master password
class NewMaster:public QDialog{
public:
	NewMaster(const std::string& = "");
	std::string password()const;
private:
	QLineEdit *first;
	QLineEdit *second;
	std::string master;
};

class AddPassword:public QDialog{
public:
	AddPassword(Manager &manager, const std::string*, const std::string*, const std::string*);
	Password password()const;
private:
	QLineEdit *name;
	QLineEdit *usrname;
	QLineEdit *pass;
};

class ViewPassword:public QDialog{
public:
	ViewPassword(const Password&, Passwords&, Manager&);
};

class Settings:public QDialog{
public:
	struct config{
		std::string master;
	};

	Settings(const config&);
	config get_config()const;

private:
	config cfg;
};

#endif // DIALOG_H
