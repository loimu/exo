cmake_minimum_required(VERSION 3.1.0)

project(exo-player LANGUAGES CXX)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 9)
set(VERSION_PATCH 97)
set(EXO_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

option(BUILD_DBUS         "Enable DBus support"           ON)
option(BUILD_LASTFM       "Enable Last.fm support"        ON)
option(USE_CMUS           "Use Cmus instead of MOC"      OFF)
option(BUILD_TESTS        "Build tests"                  OFF)
option(BUILD_TRANSLATIONS "Build translations"            ON)

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(tools)
ADD_SUBDIRECTORY(misc)
