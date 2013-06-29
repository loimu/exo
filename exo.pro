QT += network

TEMPLATE = app
TARGET = exo

INCLUDEPATH += . src

# Input
HEADERS += src/exo.h src/lyricswindow.h src/playerinterface.h src/trayicon.h
FORMS += src/about.ui src/lyricswindow.ui
SOURCES += src/exo.cpp \
           src/lyricswindow.cpp \
           src/main.cpp \
           src/playerinterface.cpp \
           src/trayicon.cpp
RESOURCES += exo.qrc
