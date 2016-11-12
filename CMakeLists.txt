project(exo)
cmake_minimum_required(VERSION 3.1.0)

option(BUILD_WITH_QT4   "Build with Qt4 by default"    OFF)
option(BUILD_DBUS       "Enable DBus support"           ON)
option(BUILD_LASTFM     "Enable Last.fm support"        ON)
option(USE_CMUS         "Use Cmus instead of MOC"      OFF)

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(tools)
ADD_SUBDIRECTORY(misc)
