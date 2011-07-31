#-------------------------------------------------
#
# Project created by QtCreator 2011-07-31T16:05:05
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_qcsvfiletest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_qcsvfiletest.cpp \
    ../src/qcsvfile.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    ../src/qcsvfile.h
