#-------------------------------------------------
#
# Project created by QtCreator 2017-02-05T15:33:45
#
#-------------------------------------------------

QT       -= core gui

TARGET = BHAPI
TEMPLATE = lib

DEFINES += BHAPI_LIBRARY

SOURCES += bhapi.cpp

HEADERS += bhapi.h\
        bhapi_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
