HEADERS += Passwords.h
HEADERS += Dialog.h
HEADERS += Manager.h

SOURCES += main.cpp
SOURCES += Passwords.cpp
SOURCES += Dialog.cpp
SOURCES += Manager.cpp

CONFIG += debug console

QMAKE_CXXFLAGS += -std=c++17

QT += widgets
