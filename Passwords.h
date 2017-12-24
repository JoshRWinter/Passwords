#ifndef PASSWORDS_H
#define PASSWORDS_H

#include <QWidget>
#include <QListWidget>

#include "Manager.h"

class Passwords:public QWidget{
public:
	Passwords(Manager&);
	Passwords(const Passwords&) = delete;
	void refresh();

private:
	void add();
	void view(const QListWidgetItem*);

	QListWidget *list;

	Manager &manager;
};

#endif // PASSWORDS_H
