QT += network

TEMPLATE = app
TARGET = exo

INCLUDEPATH += . src

# Input
HEADERS += src/exo.h src/lyricswindow.h src/playerinterface.h src/trayicon.h \
    src/aboutdialog.h
FORMS += src/lyricswindow.ui \
    src/aboutdialog.ui
SOURCES += src/exo.cpp \
           src/lyricswindow.cpp \
           src/main.cpp \
           src/playerinterface.cpp \
           src/trayicon.cpp \
    src/aboutdialog.cpp
RESOURCES += exo.qrc
