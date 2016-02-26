#-------------------------------------------------
#
# Project created by QtCreator 2016-02-23T21:16:27
#
#-------------------------------------------------

QT       += core gui widgets multimedia

CONFIG += c++11

TARGET = EyeRescue
TEMPLATE = app


SOURCES += main.cpp\
        src/mainwindow.cpp \
        src/settings.cpp

HEADERS  += inc/mainwindow.h \
    inc/settings.h

FORMS    += src/mainwindow.ui

RESOURCES += \
    etc/resources.qrc

DISTFILES += \
    README.md

INCLUDEPATH += inc/
