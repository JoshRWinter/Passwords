#include <QVBoxLayout>
#include <QListWidget>

#include "Passwords.h"


Passwords::Passwords(Manager &mgr)
	:manager(mgr)
{
	setWindowTitle("PasswordsQt");
	resize(400, 600);

	QVBoxLayout *vbox = new QVBoxLayout;
	setLayout(vbox);

	QListWidget *list = new QListWidget;

	vbox->addWidget(list);
}
