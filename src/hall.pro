QT       += core gui

TARGET = hall
TEMPLATE = app

LIBS += -L/usr/local/lib -lmsdp2xxx
INCLUDEPATH += /usr/local/include

QWT_SET=0
exists(/usr/include/qwt5) {
    LIBS += -lqwt5
    INCLUDEPATH += /usr/include/qwt5
    QWT_SET=1
}

exists(/usr/include/qwt-qt4) {
    if (QWT_SET != 1) {
        LIBS += -lqwt-qt4
        INCLUDEPATH += /usr/include/qwt-qt4
        QWT_SET=1
    }
}

exists(/usr/include/qwt) {
    if (QWT_SET != 1) {
        LIBS += -lqwt
        INCLUDEPATH += /usr/include/qwt
        QWT_SET=1
    }
}

SOURCES += main.cpp\
        mainwindow.cpp \
    configui.cpp \
    powpolwitch.cpp \
    qserial.cpp \
    qcsvfile.cpp \
    config.cpp \
    experiment.cpp \
    scpi_dev.cpp

HEADERS  += mainwindow.h \
    configui.h \
    powpolswitch.h \
    qserial.h \
    qcsvfile.h \
    experiment.h \
    config.h \
    experiment.h \
    scpi_dev.h

FORMS    += mainwindow.ui \
    configui.ui
