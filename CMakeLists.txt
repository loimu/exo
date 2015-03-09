project(exo)
cmake_minimum_required(VERSION 2.8.9)

set(CMAKE_AUTOMOC ON)

option(BUILD_WITH_QT4 "Build with Qt4 by default" ON)
if( NOT BUILD_WITH_QT4 )
    find_package(Qt5Core QUIET)
endif()

set(exo_SRCS src/exo.cpp src/main.cpp
             src/aboutdialog.cpp
             src/lyricsdialog.cpp
             src/playerinterface.cpp
             src/mocplayerinterface.cpp
             src/scrobbler.cpp
             src/scrobblersettings.cpp
             src/trayicon.cpp)
set(exo_HDRS src/exo.h
             src/aboutdialog.h
             src/lyricsdialog.h
             src/playerinterface.h
             src/mocplayerinterface.h
             src/scrobbler.h
             src/scrobblersettings.h
             src/trayicon.h)
set(exo_FRMS src/aboutdialog.ui src/lyricsdialog.ui src/scrobblersettings.ui)
set(exo_RSRCS exo.qrc)

find_library(LASTFM_LIBRARIES lastfm)
if(Qt5Core_DIR)
  find_package(Qt5Widgets)
  find_package(Qt5Network)
  find_package(Qt5DBus)
  include_directories(${Qt5Core_INCLUDE_DIRS}
                      ${Qt5Widgets_INCLUDE_DIRS}
                      ${Qt5Network_INCLUDE_DIRS}
                      ${CMAKE_CURRENT_BINARY_DIR})
  qt5_wrap_ui(exo_FORMS_HEADERS ${exo_FRMS})
  qt5_add_resources(exo_RESOURCES_RCC ${exo_RSRCS})
  qt5_add_dbus_adaptor(exo_SRCS src/dbus/tk.loimu.exo.TrayIcon.xml ../src/trayicon.h TrayIcon)
  add_executable(exo ${exo_SRCS} ${exo_HEADERS_MOC} ${exo_FORMS_HEADERS} ${exo_RESOURCES_RCC})
  target_link_libraries(exo Qt5::Core Qt5::Widgets Qt5::Network Qt5::DBus ${LASTFM_LIBRARIES})
else()
  find_package(Qt4)
  include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})
  qt4_wrap_ui(exo_FORMS_HEADERS ${exo_FRMS})
  qt4_add_resources(exo_RESOURCES_RCC ${exo_RSRCS})
  qt4_add_dbus_adaptor(exo_SRCS src/dbus/tk.loimu.exo.TrayIcon.xml ../src/trayicon.h TrayIcon)
  add_executable(exo ${exo_SRCS} ${exo_HEADERS_MOC} ${exo_FORMS_HEADERS} ${exo_RESOURCES_RCC})
  target_link_libraries(exo ${QT_QTCORE_LIBRARY}    ${QT_QTGUI_LIBRARY}  ${QT_QTDBUS_LIBRARY}
                            ${QT_QTNETWORK_LIBRARY} ${LASTFM_LIBRARIES})
endif()

install(TARGETS exo RUNTIME DESTINATION bin)
