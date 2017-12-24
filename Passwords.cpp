#include <cctype>

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
	auto searchbar = new QLineEdit;
	auto add = new QPushButton("Add Password");

	QObject::connect(add, &QPushButton::clicked, this, &Passwords::add);
	QObject::connect(list, &QListWidget::itemDoubleClicked, this, &Passwords::view);
	QObject::connect(searchbar, &QLineEdit::textChanged, [this](const QString &text){
		refresh(text.toStdString());
	});

	vbox->addWidget(searchbar);
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

void Passwords::refresh(const std::string &filter){
	list->clear();

	const std::vector<Password> *all = NULL;

	const std::vector<Password> &entries = manager.get();
	std::vector<Password> sorted;
	if(filter.length() > 0){
		all = &sorted;
		for(const Password &pw : entries){
			if(Passwords::to_lower(pw.name()).find(Passwords::to_lower(filter)) != std::string::npos){
				sorted.push_back(pw);
			}
		}
	}
	else
		all = &entries;

	for(const Password &entry : *all)
		list->addItem(entry.name().c_str());
}

std::string Passwords::to_lower(const std::string &str){
	std::string lower(str);

	for(char &c : lower)
		c = tolower(c);

	return lower;
}
