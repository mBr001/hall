QT       += core gui script

TARGET = hall
TEMPLATE = app

LIBS += -L../qcsvwriter -lQCSVWriter -L/usr/local/lib -lmsdp2xxx
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
    qserial.cpp \
    config.cpp \
    experiment.cpp \
    scpi_dev.cpp

HEADERS  += mainwindow.h \
    configui.h \
    powpolswitch.h \
    qserial.h \
    experiment.h \
    config.h \
    experiment.h \
    scpi_dev.h

FORMS    += configui.ui \
    mainwindow.ui
