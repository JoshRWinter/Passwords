#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include <QTimer>

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

NewMaster::NewMaster(){
	setWindowTitle("Create a new Master Password");
	resize(300, 0);

	auto vbox = new QVBoxLayout;
	auto hbox = new QHBoxLayout;
	auto form = new QFormLayout;
	setLayout(vbox);

	auto ok = new QPushButton("OK");
	auto cancel = new QPushButton("Cancel");
	first = new QLineEdit;
	second = new QLineEdit;
	first->setEchoMode(QLineEdit::Password);
	second->setEchoMode(QLineEdit::Password);
	QObject::connect(ok, &QPushButton::clicked, [this](){
		if(first->text() != second->text()){
			QMessageBox::critical(this, "Error", "Passwords do not match!");
		}
		else{
			if(first->text().length() == 0){
				if(QMessageBox::No == QMessageBox::warning(this, "Empty Password", "Having an empty password can be very dangerous, "
					"as it allows anyone to see all of you stored passwords without authenticating.\n"
					"Are you sure you want to have an empty master password?", QMessageBox::Yes|QMessageBox::No)){
						return;
				}
			}
			master = first->text().toStdString();
			accept();
		}
	});
	QObject::connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

	form->addRow("Master Password", first);
	form->addRow("Confirm Password", second);
	hbox->addWidget(ok);
	hbox->addWidget(cancel);
	vbox->addLayout(form);
	vbox->addLayout(hbox);
}

std::string NewMaster::password()const{
	return master;
}

AddPassword::AddPassword(const std::string *nm, const std::string *pw){
	const char *const nametip = "The service that the password is associated with (e.g. Facebook)";
	const char *const passtip = "The password";

	if(nm || pw)
		setWindowTitle("Edit Password");
	else
		setWindowTitle("Add a new Password");
	resize(350, 0);

	auto form = new QFormLayout;
	auto vbox = new QVBoxLayout;
	auto hbox = new QHBoxLayout;
	setLayout(vbox);

	auto namelabel = new QLabel("Name");
	auto passlabel = new QLabel("Password");
	name = new QLineEdit;
	pass = new QLineEdit;
	name->setText(nm ? nm->c_str() : "");
	pass->setText(pw ? pw->c_str() : "");
	namelabel->setToolTip(nametip);
	passlabel->setToolTip(passtip);
	name->setToolTip(nametip);
	pass->setToolTip(passtip);
	auto ok = new QPushButton("OK");
	auto cancel = new QPushButton("Cancel");

	QObject::connect(ok, &QPushButton::clicked, [this](){
		const QString trimmed_name = name->text().trimmed();
		if(trimmed_name.length() == 0){
			QMessageBox::critical(this, "Error", "You cannot leave the Name field blank");
		}
		else
			accept();
	});

	QObject::connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

	form->addRow(namelabel, name);
	form->addRow(passlabel, pass);
	hbox->addWidget(ok);
	hbox->addWidget(cancel);
	vbox->addLayout(form);
	vbox->addLayout(hbox);
}

Password AddPassword::password()const{
	Password pw;
	pw.set_name(name->text().trimmed().toStdString());
	pw.set_password(pass->text().toStdString());

	return pw;
}

ViewPassword::ViewPassword(const Password &passwd){
	const char *const copyto = "Copy to clipboard";
	const char *const copied = "Copied";

	setWindowTitle(passwd.name().c_str());

	auto vbox = new QVBoxLayout;
	auto hbox = new QHBoxLayout;
	setLayout(vbox);

	auto namelabel = new QLabel(("Name: " + passwd.name()).c_str());
	auto passlabel = new QLabel("Password:");
	auto passfield = new QLineEdit(passwd.password().c_str());
	passfield->setReadOnly(true);
	auto copytoclipboard = new QPushButton(copyto);
	QObject::connect(copytoclipboard, &QPushButton::clicked, [this, copytoclipboard, copyto, copied]{
		copytoclipboard->setText(copied);
		QTimer::singleShot(1000, this, [copytoclipboard, copyto, copied]{
			copytoclipboard->setText(copyto);
		});
	});

	hbox->addWidget(passlabel);
	hbox->addWidget(passfield);
	vbox->addWidget(namelabel);
	vbox->addLayout(hbox);
	vbox->addWidget(copytoclipboard);
}
