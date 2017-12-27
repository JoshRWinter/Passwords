.PHONY := clean release

all: Makefile.qmake
	make -f Makefile.qmake
	./passwords

Makefile.qmake: passwords.pro
	qmake -o Makefile.qmake

release: Makefile.qmake clean
	g++ -o passwords -Wall -pedantic -O2 -std=c++17 -fpic `pkg-config --cflags Qt5Widgets` *.cpp -s -lcrypto `pkg-config --libs Qt5Widgets`

clean:
	make -f Makefile.qmake distclean
