#-------------------------------------------------
#
# Project created by QtCreator 2018-12-20T19:41:37
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = snakeGUI
TEMPLATE = app


SOURCES += src/main.cpp\
           src/mainwindow.cpp \
           src/scribblearea.cpp \
    src/cChamferDistance.cpp \
    src/cImage.cpp \
    src/cPoint.cpp \
    src/cSnake.cpp \
    src/gradflow.cpp

HEADERS  += include/mainwindow.h \
            include/scribblearea.h \
    include/c2DArray.h \
    include/cChamferDistance.h \
    include/cImage.h \
    include/cOutOfBoundException.h \
    include/cPoint.h \
    include/cSnake.h \
    include/gradflow.h

FORMS    += mainwindow.ui

QMAKE_CXXFLAGS += "-Wno-expansion-to-defined"
