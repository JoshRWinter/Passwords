all: Makefile.qmake
	make -f Makefile.qmake
	./passwords

Makefile.qmake: passwords.pro
	qmake -o Makefile.qmake
