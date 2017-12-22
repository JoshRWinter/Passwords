#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QLineEdit>

class Greeter:public QDialog{
public:
	Greeter();
	std::string password()const;
private:
	QLineEdit *pass;
};

#endif // DIALOG_H
