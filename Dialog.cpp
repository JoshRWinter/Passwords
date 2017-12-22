#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

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
