#ifndef PASSWORDS_H
#define PASSWORDS_H

#include <QWidget>
#include <QListWidget>

#include "Manager.h"

class Passwords:public QWidget{
public:
	Passwords(Manager&);
	Passwords(const Passwords&) = delete;
	void refresh(const std::string& = "");

private:
	void add();
	void view(const QListWidgetItem*);
	static std::string to_lower(const std::string&);

	QListWidget *list;

	Manager &manager;
};

#endif // PASSWORDS_H
