#-------------------------------------------------
#
# Project created by QtCreator 2017-08-23T17:15:27
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Lora_RT
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mylora.cpp

HEADERS  += mainwindow.h \
    lora_pindef.h \
    lora_regdef.h \
    mylora.h

FORMS    += mainwindow.ui
LIBS +=-L/usr/local/lib/include -lwiringPi

