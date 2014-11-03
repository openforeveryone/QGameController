#-------------------------------------------------
#
# Project created by QtCreator 2014-10-28T13:43:22
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QJoystick
TEMPLATE = app

win32:LIBS      += -ldinput8 -ldxguid

LIBS += -framework IOKit
LIBS += -framework CoreFoundation

SOURCES += main.cpp\
        widget.cpp \
    qjoystick.cpp

HEADERS  += widget.h \
    qjoystick.h

FORMS    +=

win32:CONFIG += c++11
