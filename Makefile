.PHONY := clean release install uninstall

all: Makefile.qmake
	make -f Makefile.qmake
	./passwords

Makefile.qmake: passwords.pro
	qmake -o Makefile.qmake

release: Makefile.qmake clean
	g++ -o passwords -Wall -pedantic -O2 -std=c++17 -fpic `pkg-config --cflags Qt5Widgets` *.cpp -s -lcrypto `pkg-config --libs Qt5Widgets`

clean:
	make -f Makefile.qmake distclean

install:
	if [ ! -d /usr/share/Passwords ] ; then \
		sudo mkdir /usr/share/Passwords; \
	fi
	sudo cp american-english /usr/share/Passwords
	sudo cp passwords.desktop /usr/share/applications
	sudo cp passwords /usr/bin/

uninstall:
	sudo rm /usr/share/Passwords/american-english
	sudo rmdir /usr/share/Passwords
	sudo rm /usr/bin/passwords
	sudo rm /usr/share/applications/passwords.desktop
