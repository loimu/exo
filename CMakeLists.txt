project(exo)
cmake_minimum_required(VERSION 3.1.0)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 7)
set(VERSION_PATCH 97)
set(EXO_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

option(BUILD_WITH_QT4   "Build with Qt4 by default"    OFF)
option(BUILD_DBUS       "Enable DBus support"           ON)
option(BUILD_LASTFM     "Enable Last.fm support"        ON)
option(USE_CMUS         "Use Cmus instead of MOC"      OFF)

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(tools)
ADD_SUBDIRECTORY(misc)
