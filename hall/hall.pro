QT       += core gui script

TARGET = hall
TEMPLATE = app

LIBS += -L../QCSVWriter -lQCSVWriter \
    -L../QSCPIDev -lQSCPIDev \
    -L/usr/local/lib -lmsdp2xxx
INCLUDEPATH += /usr/local/include

# expected QWT >= 6.0.0
LIBS += -L/usr/local/lib -lqwt
INCLUDEPATH += /usr/local/include/qwt /usr/include/qwt
CONFIG += qwt

SOURCES += main.cpp\
        mainwindow.cpp \
    configui.cpp \
    powpolwitch.cpp \
    config.cpp \
    experiment.cpp

HEADERS  += mainwindow.h \
    configui.h \
    powpolswitch.h \
    config.h \
    experiment.h

FORMS    += configui.ui \
    mainwindow.ui
