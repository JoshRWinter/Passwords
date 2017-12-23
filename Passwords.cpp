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
	auto hbox = new QHBoxLayout;
	setLayout(vbox);

	list = new QListWidget;
	auto add = new QPushButton("New");
	auto edit = new QPushButton("Edit");
	auto remove = new QPushButton("Remove");

	QObject::connect(add, &QPushButton::clicked, this, &Passwords::add);
	QObject::connect(edit, &QPushButton::clicked, this, &Passwords::edit);
	QObject::connect(remove, &QPushButton::clicked, this, &Passwords::remove);

	hbox->addWidget(add);
	hbox->addWidget(edit);
	hbox->addWidget(remove);
	vbox->addWidget(list);
	vbox->addLayout(hbox);

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

void Passwords::edit(){
	try{
		auto item = list->currentItem();
		if(item == NULL)
			return;

		const Password &pw = manager.find(item->text().toStdString());
		const std::string name = pw.name();
		const std::string pass = pw.password();
		AddPassword editpass(&name, &pass);
		if(editpass.exec()){
			const Password &passwd = editpass.password();
			manager.edit(name, passwd.name(), passwd.password());
			refresh();
		}
	}catch(const Manager::ManagerException &e){
		QMessageBox::critical(this, "Database Error", e.what());
	}
}

void Passwords::remove(){
	auto item = list->currentItem();
	if(item == NULL)
		return;

	if(QMessageBox::question(this, "Remove Item?", "Are you sure you want to remove this item?") == QMessageBox::Yes){
		try{
			manager.remove(item->text().toStdString());
			refresh();
		}catch(const Manager::ManagerException &e){
			QMessageBox::critical(this, "Database Error", e.what());
		}
	}
}

void Passwords::refresh(){
	list->clear();

	const std::vector<Password> entries = manager.get();
	for(const Password &entry : entries)
		list->addItem(entry.name().c_str());
}
