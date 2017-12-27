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
	NewMaster();
	std::string password()const;
private:
	QLineEdit *first;
	QLineEdit *second;
	std::string master;
};

class AddPassword:public QDialog{
public:
	AddPassword(Manager &manager, const std::string*, const std::string*);
	Password password()const;
private:
	QLineEdit *name;
	QLineEdit *pass;
};

class ViewPassword:public QDialog{
public:
	ViewPassword(const Password&, Passwords&, Manager&);
};

#endif // DIALOG_H
