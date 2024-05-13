cmake_minimum_required(VERSION 3.5)
project(exo)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 9)
set(VERSION_PATCH 98)
set(EXO_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

file(READ rev EXO_REVISION LIMIT 9)

option(BUILD_DBUS         "Enable DBus support"           ON)
option(BUILD_LASTFM       "Enable Last.fm support"        ON)
option(BUILD_CMUS         "Enable Cmus interface"        OFF)
option(BUILD_TESTS        "Build tests"                  OFF)
option(WITH_QT6           "Build with Qt6"               OFF)

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(tools)
ADD_SUBDIRECTORY(misc)
