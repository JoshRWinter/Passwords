#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QLineEdit>

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

#endif // DIALOG_H
