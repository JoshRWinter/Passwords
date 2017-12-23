#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>

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
