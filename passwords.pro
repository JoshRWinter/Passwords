HEADERS += Passwords.h
HEADERS += Dialog.h
HEADERS += Manager.h
HEADERS += crypto.h

SOURCES += main.cpp
SOURCES += Passwords.cpp
SOURCES += Dialog.cpp
SOURCES += Manager.cpp
SOURCES += crypto.cpp

CONFIG += debug console

QMAKE_CXXFLAGS += -std=c++17
QMAKE_LFLAGS += -lcrypto

QT += widgets
