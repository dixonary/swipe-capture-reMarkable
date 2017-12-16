TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

TARGET=swipe-capture

target.path = /usr/bin
INSTALLS += target
