QT       += core gui script

TARGET = hall
TEMPLATE = app

LIBS += -L../qcsvwriter -lQCSVWriter \
    -L../QSCPIDev -lQSCPIDev \
    -L/usr/local/lib -lmsdp2xxx
INCLUDEPATH += /usr/local/include

exists(/usr/include/qwt5) {
    LIBS += -lqwt5
    INCLUDEPATH += /usr/include/qwt5
    CONFIG += qwt
}

exists(/usr/include/qwt-qt4) {
    !CONFIG(qwt) {
        LIBS += -lqwt-qt4
        INCLUDEPATH += /usr/include/qwt-qt4
        CONFIG += qwt
    }
}

exists(/usr/include/qwt) {
    !CONFIG(qwt) {
        LIBS += -lqwt
        INCLUDEPATH += /usr/include/qwt
        CONFIG += qwt
    }
}

SOURCES += main.cpp\
        mainwindow.cpp \
    configui.cpp \
    powpolwitch.cpp \
    config.cpp \
    experiment.cpp

HEADERS  += mainwindow.h \
    configui.h \
    powpolswitch.h \
    experiment.h \
    config.h \
    experiment.h

FORMS    += configui.ui \
    mainwindow.ui
