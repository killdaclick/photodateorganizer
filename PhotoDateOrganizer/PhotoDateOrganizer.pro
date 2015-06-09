#-------------------------------------------------
#
# Project created by QtCreator 2015-06-04T11:06:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = "Photo Date Organizer"
TEMPLATE = app

SOURCES += main.cpp\
        MainWindow.cpp \
        AboutWindow.cpp \
        Utility.cpp

HEADERS  += MainWindow.h \
			AboutWindow.h \
			Utility.h

FORMS    += MainWindow.ui \
			AboutWindow.ui
			
RESOURCES     = resource.qrc

win32:RC_ICONS += icons/app/ImageCapture-icon.ico