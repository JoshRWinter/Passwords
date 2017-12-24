#ifndef PASSWORDS_H
#define PASSWORDS_H

#include <QWidget>
#include <QListWidget>

#include "Manager.h"

class Passwords:public QWidget{
public:
	Passwords(Manager&);

private:
	void add();
	void edit();
	void remove();
	void view(const QListWidgetItem*);
	void refresh();

	QListWidget *list;

	Manager &manager;
};

#endif // PASSWORDS_H
