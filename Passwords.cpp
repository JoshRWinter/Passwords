#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>

#include "Passwords.h"
#include "Dialog.h"

Passwords::Passwords(Manager &mgr)
	:manager(mgr)
{
	setWindowTitle("PasswordsQt");
	resize(400, 600);

	auto vbox = new QVBoxLayout;
	setLayout(vbox);

	list = new QListWidget;
	auto add = new QPushButton("Add Password");

	QObject::connect(add, &QPushButton::clicked, this, &Passwords::add);
	QObject::connect(list, &QListWidget::itemDoubleClicked, this, &Passwords::view);

	vbox->addWidget(list);
	vbox->addWidget(add);

	refresh();
}

void Passwords::add(){
	AddPassword newpass(NULL, NULL);
	if(newpass.exec()){
		Password pw = newpass.password();
		try{
			manager.add(pw);
		}catch(const Manager::ManagerException &e){
			QMessageBox::critical(this, "Database Error", e.what());
			add(); // recurse
		}
	}

	refresh();
}

void Passwords::view(const QListWidgetItem *item){
	const Password &passwd = manager.find(item->text().toStdString());
	ViewPassword vp(passwd, *this, manager);
	vp.exec();
}

void Passwords::refresh(){
	list->clear();

	const std::vector<Password> entries = manager.get();
	for(const Password &entry : entries)
		list->addItem(entry.name().c_str());
}
