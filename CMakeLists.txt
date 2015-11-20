project(exo)
cmake_minimum_required(VERSION 2.8.9)

option(BUILD_WITH_QT4   "Build with Qt4 by default"     ON)
option(BUILD_DBUS       "Enable DBus support"           ON)
option(BUILD_LASTFM     "Enable Last.fm support"        ON)
option(USE_CMUS         "Use Cmus instead of MOC"      OFF)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(CMAKE_AUTOMOC ON)

set(exo_SRCS src/exo.cpp src/main.cpp
             src/aboutdialog.cpp
             src/lyricsdialog.cpp
             src/bookmark.cpp
             src/bookmarkmanager.cpp
             src/mocplayerinterface.cpp
             src/playerinterface.cpp
             src/trayicon.cpp)
set(exo_FRMS src/aboutdialog.ui src/lyricsdialog.ui)
set(exo_RSRCS exo.qrc)

if(USE_CMUS)
  list(APPEND exo_SRCS src/cmusinterface.cpp)
endif()

if(NOT BUILD_WITH_QT4)
  find_package(Qt5Core QUIET)
endif()

if(BUILD_DBUS)
  list(APPEND exo_SRCS src/dbus/dbus.cpp src/dbus/exoobject.cpp
                       src/dbus/rootobject.cpp src/dbus/playerobject.cpp)
endif()

if(BUILD_LASTFM)
  list(APPEND exo_SRCS src/scrobbler.cpp src/scrobblersettings.cpp)
  list(APPEND exo_FRMS src/scrobblersettings.ui)
  find_library(LASTFM_LIBRARIES lastfm)
endif()

if(Qt5Core_DIR)
  find_package(Qt5Widgets)
  find_package(Qt5Network)
  set(qt_LIBS Qt5::Core Qt5::Widgets Qt5::Network)
  if(BUILD_DBUS)
    find_package(Qt5DBus)
    list(APPEND qt_LIBS Qt5::DBus)
  endif()
  include_directories(${Qt5Core_INCLUDE_DIRS}
                      ${Qt5Widgets_INCLUDE_DIRS}
                      ${Qt5Network_INCLUDE_DIRS}
                      ${CMAKE_CURRENT_SOURCE_DIR}/dbus
                      ${CMAKE_CURRENT_BINARY_DIR})
  qt5_wrap_ui(exo_FORMS_HEADERS ${exo_FRMS})
  qt5_add_resources(exo_RESOURCES_RCC ${exo_RSRCS})
else()
  find_package(Qt4)
  set(qt_LIBS Qt4::QtCore Qt4::QtGui Qt4::QtNetwork)
  if(BUILD_DBUS)
    list(APPEND qt_LIBS Qt4::QtDBus)
  endif()
  include_directories(${QT_INCLUDES}
                      ${CMAKE_CURRENT_SOURCE_DIR}/dbus
                      ${CMAKE_CURRENT_BINARY_DIR})
  qt4_wrap_ui(exo_FORMS_HEADERS ${exo_FRMS})
  qt4_add_resources(exo_RESOURCES_RCC ${exo_RSRCS})
endif()

add_executable(exo ${exo_SRCS} ${exo_HEADERS_MOC}
                   ${exo_FORMS_HEADERS} ${exo_RESOURCES_RCC})
target_link_libraries(exo ${qt_LIBS} ${LASTFM_LIBRARIES})

install(TARGETS exo RUNTIME DESTINATION bin)
