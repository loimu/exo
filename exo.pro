QT += network

TEMPLATE = app
TARGET = exo

target.path = /usr/local/bin/
INSTALLS += target

#INCLUDEPATH += . src
LIBS += /usr/lib/i386-linux-gnu/liblastfm.so

# Input
HEADERS += src/exo.h src/lyricswindow.h src/playerinterface.h src/trayicon.h \
    src/scrobbler.h \
    src/aboutdialog.h \
    src/scrobblersettings.h

FORMS += src/lyricswindow.ui \
    src/aboutdialog.ui \
    src/scrobblersettings.ui
SOURCES += src/exo.cpp \
           src/lyricswindow.cpp \
           src/main.cpp \
           src/playerinterface.cpp \
           src/trayicon.cpp \
    src/scrobbler.cpp \
    src/aboutdialog.cpp \
    src/scrobblersettings.cpp
RESOURCES += exo.qrc
