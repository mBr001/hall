#-------------------------------------------------
#
# Project created by QtCreator 2011-07-07T08:10:15
#
#-------------------------------------------------

QT       += core gui

TARGET = hall
TEMPLATE = app

LIBS += -L/usr/local/lib -lmsdp2xxx
INCLUDEPATH += /usr/local/include

exists(/usr/include/qwt) {
    LIBS += -lqwt
    INCLUDEPATH += /usr/include/qwt
}

exists(/usr/include/qwt-qt4) {
    !exists(/usr/include/qwt) {
        LIBS += -lqwt-qt4
        INCLUDEPATH += /usr/include/qwt-qt4
    }
}

SOURCES += main.cpp\
        mainwindow.cpp \
    configui.cpp \
    powpolwitch.cpp \
    ps6220hack.cpp \
    hp34970hack.cpp \
    qserial.cpp \
    qcsvfile.cpp \
    config.cpp \
    error.cpp \
    experiment.cpp

HEADERS  += mainwindow.h \
    configui.h \
    powpolswitch.h \
    ps6220hack.h \
    hp34970hack.h \
    qserial.h \
    qcsvfile.h \
    experiment.h \
    config.h \
    error.h \
    experiment.h

FORMS    += mainwindow.ui \
    configui.ui
