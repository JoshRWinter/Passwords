#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include <QTimer>
#include <QSpacerItem>
#include <QClipboard>
#include <QApplication>

#include "Dialog.h"

Greeter::Greeter(){
	setWindowTitle("Input Master Password");

	auto form = new QFormLayout;
	auto vbox = new QVBoxLayout;
	auto hbox = new QHBoxLayout;
	setLayout(vbox);
	resize(300, 0);

	pass = new QLineEdit();
	auto ok = new QPushButton("OK");
	auto cancel = new QPushButton("Cancel");

	pass->setEchoMode(QLineEdit::Password);
	QObject::connect(ok, &QPushButton::clicked, this, &QDialog::accept);
	QObject::connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

	form->addRow("Master Password", pass);
	hbox->addWidget(ok);
	hbox->addWidget(cancel);
	vbox->addLayout(form);
	vbox->addLayout(hbox);
}

std::string Greeter::password()const{
	return pass->text().toStdString();
}

NewMaster::NewMaster(const std::string &current){
	setWindowTitle("Create a new Master Password");
	resize(300, 0);

	const char *const helptext=
	"You will now create a Master Password.\n\n"
	"Your Master Password guards access to your entire password "
	"database. It will need to be a strong password, but also "
	"memorable. Try to create one that is at least 12 characters long.\n\n"
	"There is no way to recover your Master Password should you forget "
	"it!\n";

	auto vbox = new QVBoxLayout;
	auto hbox = new QHBoxLayout;
	auto form = new QFormLayout;
	setLayout(vbox);

	auto help = new QLabel(helptext);
	help->setWordWrap(true);
	help->setMaximumWidth(400);
	auto currentpass = new QLineEdit;
	auto ok = new QPushButton("OK");
	auto cancel = new QPushButton("Cancel");
	first = new QLineEdit;
	second = new QLineEdit;
	first->setEchoMode(QLineEdit::Password);
	second->setEchoMode(QLineEdit::Password);
	QObject::connect(ok, &QPushButton::clicked, [this, &current, currentpass](){
		if(first->text() != second->text()){
			QMessageBox::critical(this, "Error", "Passwords do not match!");
		}
		else if(current.size() > 0 && current != currentpass->text().toStdString()){
			QMessageBox::critical(this, "Error", "Incorrect current master password!");
		}
		else{
			if(first->text().length() == 0){
				if(QMessageBox::No == QMessageBox::warning(this, "Empty Password", "Having an empty password can be very dangerous, "
					"as it allows anyone to see all of your stored passwords without authenticating.\n\n"
					"Are you sure you want to have an empty master password?", QMessageBox::Yes|QMessageBox::No)){
						return;
				}
			}
			master = first->text().toStdString();
			accept();
		}
	});
	QObject::connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

	if(current.size() > 0){
		currentpass->setEchoMode(QLineEdit::Password);
		form->addRow("Current Master Password", currentpass);
	}
	else
		delete currentpass;

	form->addRow("Master Password", first);
	form->addRow("Confirm Password", second);
	hbox->addWidget(ok);
	hbox->addWidget(cancel);
	vbox->addWidget(help);
	vbox->addLayout(form);
	vbox->addLayout(hbox);

	setMinimumSize(sizeHint());
}

std::string NewMaster::password()const{
	return master;
}

AddPassword::AddPassword(Manager &manager, const std::string *nm, const std::string *un, const std::string *pw){
	const char *const nametip = "The service that the password is associated with (e.g. Facebook)";
	const char *const usrtip = "The user name";
	const char *const passtip = "The password";

	if(nm || un || pw)
		setWindowTitle("Edit Password");
	else
		setWindowTitle("Add a new Password");
	resize(350, 0);

	auto form = new QFormLayout;
	auto vbox = new QVBoxLayout;
	auto hbox1 = new QHBoxLayout;
	auto hbox2 = new QHBoxLayout;
	setLayout(vbox);

	auto namelabel = new QLabel("Description");
	auto usrnamelabel = new QLabel("User name");
	auto passlabel = new QLabel("Password");
	name = new QLineEdit;
	usrname = new QLineEdit;
	pass = new QLineEdit;
	name->setText(nm ? nm->c_str() : "");
	usrname->setText(un ? un->c_str() : "");
	pass->setText(pw ? pw->c_str() : "");
	namelabel->setToolTip(nametip);
	usrnamelabel->setToolTip(usrtip);
	passlabel->setToolTip(passtip);
	name->setToolTip(nametip);
	usrname->setToolTip(usrtip);
	pass->setToolTip(passtip);
	auto ok = new QPushButton("OK");
	auto cancel = new QPushButton("Cancel");
	auto genrandom = new QPushButton("Generate Random");
	auto genmemorable = new QPushButton("Generate Memorable");

	QObject::connect(ok, &QPushButton::clicked, [this](){
		const QString trimmed_name = name->text().trimmed();
		if(trimmed_name.length() == 0){
			QMessageBox::critical(this, "Error", "You cannot leave the Description field blank");
		}
		else
			accept();
	});

	QObject::connect(genrandom, &QPushButton::clicked, [this, &manager]{
		pass->setText(Manager::gen_random().c_str());
	});

	QObject::connect(genmemorable, &QPushButton::clicked, [this, &manager]{
		try{
			pass->setText(manager.gen_memorable().c_str());
		}catch(const Manager::ManagerException &e){
			QMessageBox::critical(this, "Error", e.what());
		}
	});

	QObject::connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

	form->addRow(namelabel, name);
	form->addRow(usrnamelabel, usrname);
	form->addRow(passlabel, pass);
	hbox1->addWidget(genrandom);
	hbox1->addWidget(genmemorable);
	hbox2->addWidget(ok);
	hbox2->addWidget(cancel);
	vbox->addLayout(form);
	vbox->addLayout(hbox1);
	vbox->addLayout(hbox2);

	if(nm == NULL && pw == NULL)
		ok->setDefault(true);
	else
		cancel->setDefault(true);
}

Password AddPassword::password()const{
	Password pw;
	pw.set_name(name->text().trimmed().toStdString());
	pw.set_username(usrname->text().trimmed().toStdString());
	pw.set_password(pass->text().toStdString());

	return pw;
}

ViewPassword::ViewPassword(const Password &passwd, Passwords &parent, Manager &manager){
	const char *const copyto = "Copy to clipboard";
	const char *const copied = "Copied";

	resize(350, 0);
	setWindowTitle(passwd.name().c_str());

	auto vbox = new QVBoxLayout;
	auto hboxusername = new QHBoxLayout;
	auto hboxpassword = new QHBoxLayout;
	auto editdelete = new QHBoxLayout;
	setLayout(vbox);

	auto namelabel = new QLabel(("Description: " + passwd.name()).c_str());
	auto usrnamelabel = new QLabel("User name:");
	auto passlabel = new QLabel("Password:");
	auto usrnamefield = new QLineEdit(passwd.username().c_str());
	usrnamefield->setReadOnly(true);
	auto passfield = new QLineEdit(passwd.password().c_str());
	passfield->setReadOnly(true);
	auto copytoclipboard = new QPushButton(copyto);
	copytoclipboard->setToolTip("Copy the password to the clipboard");
	auto edit = new QPushButton("Edit");
	auto remove = new QPushButton("Delete");

	QObject::connect(copytoclipboard, &QPushButton::clicked, [this, copytoclipboard, copyto, copied, passfield]{
		copytoclipboard->setText(copied);
		QClipboard *clip = QApplication::clipboard();
		clip->setText(passfield->text());
		QTimer::singleShot(5000, this, [copytoclipboard, copyto, copied, clip]{
			copytoclipboard->setText(copyto);
			clip->clear();
		});
	});

	QObject::connect(edit, &QPushButton::clicked, [this, &passwd, &manager, &parent, namelabel, usrnamefield, passfield]{
		try{
			const std::string name = passwd.name();
			const std::string username = passwd.username();
			const std::string pass = passwd.password();
			AddPassword editpass(manager, &name, &username, &pass);
			if(editpass.exec()){
				const Password &pass = editpass.password();
				manager.edit(name, pass.name(), pass.username(), pass.password());
				namelabel->setText(("Description: " + pass.name()).c_str());
				usrnamefield->setText(pass.username().c_str());
				passfield->setText(pass.password().c_str());
				this->setWindowTitle(pass.name().c_str());
				parent.refresh();
			}
		}catch(const Manager::ManagerException &e){
			QMessageBox::critical(this, "Database Error", e.what());
		}
	});

	QObject::connect(remove, &QPushButton::clicked, [this, &parent, &manager, &passwd]{
		if(QMessageBox::question(this, "Remove Item?", "Are you sure you want to remove this item?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes){
			try{
				manager.remove(passwd.name());
				parent.refresh();
			}catch(const Manager::ManagerException &e){
				QMessageBox::critical(this, "Database Error", e.what());
			}

			accept();
		}
	});

	hboxusername->addWidget(usrnamelabel);
	hboxusername->addWidget(usrnamefield);
	hboxpassword->addWidget(passlabel);
	hboxpassword->addWidget(passfield);
	editdelete->addWidget(edit);
	editdelete->addWidget(remove);
	vbox->addWidget(namelabel);
	vbox->addLayout(hboxusername);
	vbox->addLayout(hboxpassword);
	vbox->addItem(new QSpacerItem(0, 20));
	vbox->addWidget(copytoclipboard);
	vbox->addLayout(editdelete);
}

Settings::Settings(const Settings::config &c){
	resize(300, 0);
	setWindowTitle("Settings");

	auto vbox = new QVBoxLayout;
	setLayout(vbox);

	auto chmaster = new QPushButton("Change Master Password");

	QObject::connect(chmaster, &QPushButton::clicked, [this, &c, chmaster]{
		NewMaster newm(c.master);
		if(newm.exec()){
			cfg.master = newm.password();
			accept();
		}
	});

	vbox->addWidget(chmaster);
}

Settings::config Settings::get_config()const{
	return cfg;
}
