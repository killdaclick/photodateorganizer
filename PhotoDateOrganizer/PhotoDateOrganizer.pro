#-------------------------------------------------
#
# Project created by QtCreator 2015-06-04T11:06:49
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = "Photo Date Organizer"
TEMPLATE = app

SOURCES += main.cpp\
        MainWindow.cpp \
        AboutWindow.cpp \
        Utility.cpp \
        ChangeLanguage.cpp \
        Preferences.cpp \
        FileDownloader.cpp

HEADERS  += MainWindow.h \
			AboutWindow.h \
			Utility.h \
			ChangeLanguage.h \
			Preferences.h \
			FileDownloader.h

FORMS    += MainWindow.ui \
			AboutWindow.ui \
			ChangeLanguage.ui
			
TRANSLATIONS = photodateorganizer_en.ts
			
RESOURCES     = resource.qrc

win32:RC_ICONS += icons/app/ImageCapture-icon.ico