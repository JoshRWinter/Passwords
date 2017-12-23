#ifndef PASSWORDS_H
#define PASSWORDS_H

#include <QWidget>

#include "Manager.h"

class Passwords:public QWidget{
public:
	Passwords(Manager&);

private:
	Manager &manager;
};

#endif // PASSWORDS_H
