QT       += core xml
QT       -= gui

TARGET    = rcc

CONFIG += console
CONFIG += static
CONFIG -= app_bundle
CONFIG += release

TEMPLATE = app

SOURCES += main.cpp \
    reverse.cpp \
    rcc.cpp

HEADERS += rcc.h \
    reverse.h    

RC_FILE = rcc.rc
