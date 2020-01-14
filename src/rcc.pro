QT += core xml
QT -= gui

TARGET = rcc

CONFIG += console
CONFIG += static
CONFIG -= app_bundle
CONFIG += release

TEMPLATE = app

DEFINES += QT_RCC QT_NO_FOREACH
DEFINES += QT_FEATURE_zstd=-1
DEFINES += STATIC

include(rcc.pri)
SOURCES += main.cpp

RC_FILE = rcc.rc
