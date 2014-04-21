project(exo)
cmake_minimum_required(VERSION 2.8.9)
find_package(Qt4)
find_library(LASTFM_LIBRARIES lastfm)
if(Qt4_FOUND)
  include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})
else()
  find_package(Qt5Core)
  find_package(Qt5Widgets)
  find_package(Qt5Network)
  include_directories(${Qt5Core_INCLUDE_DIRS}
                      ${Qt5Widgets_INCLUDE_DIRS}
                      ${Qt5Network_INCLUDE_DIRS}
                      ${CMAKE_CURRENT_BINARY_DIR})
endif()

set(exo_SRCS src/exo.cpp src/main.cpp
             src/aboutdialog.cpp
             src/lyricsdialog.cpp
             src/playerinterface.cpp
             src/scrobbler.cpp
             src/scrobblersettings.cpp
             src/trayicon.cpp)
set(exo_HDRS src/exo.h
             src/aboutdialog.h
             src/lyricsdialog.h
             src/playerinterface.h
             src/scrobbler.h
             src/scrobblersettings.h
             src/trayicon.h)
set(exo_FRMS src/aboutdialog.ui src/lyricsdialog.ui src/scrobblersettings.ui)
set(exo_RSRCS exo.qrc)
set(CMAKE_AUTOMOC ON)

if(Qt4_FOUND)
  qt4_wrap_ui(exo_FORMS_HEADERS ${exo_FRMS})
  qt4_add_resources(exo_RESOURCES_RCC ${exo_RSRCS})
  add_executable(exo ${exo_SRCS}
                     ${exo_HEADERS_MOC}
                     ${exo_FORMS_HEADERS}
                     ${exo_RESOURCES_RCC})
  target_link_libraries(exo ${QT_QTCORE_LIBRARY}
                            ${QT_QTGUI_LIBRARY}
                            ${QT_QTNETWORK_LIBRARY}
                            ${LASTFM_LIBRARIES})
else()
  qt5_wrap_ui(exo_FORMS_HEADERS ${exo_FRMS})
  qt5_add_resources(exo_RESOURCES_RCC ${exo_RSRCS})
  add_executable(exo ${exo_SRCS}
                     ${exo_HEADERS_MOC}
                     ${exo_FORMS_HEADERS}
                     ${exo_RESOURCES_RCC})
  target_link_libraries(exo Qt5::Core
                            Qt5::Widgets
                            Qt5::Network
                            ${LASTFM_LIBRARIES})
endif()

install(TARGETS exo RUNTIME DESTINATION bin)
