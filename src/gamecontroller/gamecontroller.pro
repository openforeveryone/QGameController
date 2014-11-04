

QT = core
load(qt_build_config)
TARGET = QtGameController
load(qt_module)

#QMAKE_DOCS = $$PWD/doc/qtserialport.qdocconf

win32:LIBS += -ldinput8 -ldxguid
win32:CONFIG += c++11

mac:LIBS += -framework IOKit
mac:LIBS += -framework CoreFoundation

SOURCES +=  qgamecontroller.cpp

PUBLIC_HEADERS += qgamecontroller.h
PRIVATE_HEADERS +=  qgamecontroller_p.h
HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS

