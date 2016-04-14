
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(CMAKE_AUTOMOC ON)

set(exo_SRCS exo.cpp main.cpp
             aboutdialog.cpp
             lyricsdialog.cpp
             bookmarks/bookmark.cpp
             bookmarks/bookmarkdialog.cpp
             bookmarks/bookmarkmanager.cpp
             mocplayerinterface.cpp
             playerinterface.cpp
             trayicon.cpp)
set(exo_FRMS aboutdialog.ui lyricsdialog.ui)
set(exo_RSRCS exo.qrc)

if(USE_CMUS)
  list(APPEND exo_SRCS cmusinterface.cpp)
endif()

if(NOT BUILD_WITH_QT4)
  find_package(Qt5Core QUIET)
endif()

if(BUILD_DBUS)
  list(APPEND exo_SRCS dbus/dbus.cpp dbus/exoobject.cpp
                       dbus/rootobject.cpp dbus/playerobject.cpp)
endif()

if(BUILD_LASTFM)
  list(APPEND exo_SRCS scrobbler.cpp scrobblersettings.cpp)
  list(APPEND exo_FRMS scrobblersettings.ui)
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
