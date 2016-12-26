
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(CMAKE_AUTOMOC ON)

set(exo_SRCS
  main.cpp
  bookmarks/bookmark.cpp
  bookmarks/bookmarkdialog.cpp
  bookmarks/bookmarkmanager.cpp
  core/exo.cpp
  core/mocinterface.cpp
  core/playerinterface.cpp
  core/process.cpp
  core/singleinstance.cpp
  gui/aboutdialog.cpp
  gui/basedialog.cpp
  gui/lyricsdialog.cpp
  gui/tageditor.cpp
  gui/trayicon.cpp
)
set(exo_FRMS gui/lyricsdialog.ui)
set(exo_RSRCS ../images/exo.qrc)

if(USE_CMUS)
  list(APPEND exo_SRCS core/cmusinterface.cpp)
endif()

if(BUILD_DBUS)
  list(APPEND exo_SRCS dbus/dbus.cpp dbus/exoobject.cpp
                       dbus/rootobject.cpp dbus/playerobject.cpp)
endif()

if(BUILD_LASTFM)
  list(APPEND exo_SRCS lastfm/scrobbler.cpp lastfm/scrobblerauth.cpp
                       core/consoleauth.cpp
                       gui/scrobblersettings.cpp)
  list(APPEND exo_FRMS gui/scrobblersettings.ui)
    if(BUILD_WITH_QT4)
      find_library(LASTFM_LIBRARIES lastfm)
    else()
      find_library(LASTFM_LIBRARIES lastfm5)
    endif()
endif()

if(BUILD_WITH_QT4)
  find_package(Qt4 4.7.2 REQUIRED QtCore QtGui QtNetwork QtXml)
  set(qt_LIBS Qt4::QtCore Qt4::QtGui Qt4::QtNetwork Qt4::QtXml)
  if(BUILD_DBUS)
    list(APPEND qt_LIBS Qt4::QtDBus)
  endif()
  include_directories(${QT_INCLUDES})
  qt4_wrap_ui(exo_FORMS_HEADERS ${exo_FRMS})
  qt4_add_resources(exo_RESOURCES_RCC ${exo_RSRCS})
else()
  find_package(Qt5 5.6.1 REQUIRED COMPONENTS Core Network Widgets Xml)
  set(qt_LIBS Qt5::Core Qt5::Network Qt5::Widgets Qt5::Xml)
  if(BUILD_DBUS)
    find_package(Qt5DBus)
    list(APPEND qt_LIBS Qt5::DBus)
  endif()
  include_directories(${Qt5Core_INCLUDE_DIRS}
                      ${Qt5Widgets_INCLUDE_DIRS}
                      ${Qt5Network_INCLUDE_DIRS})
  qt5_wrap_ui(exo_FORMS_HEADERS ${exo_FRMS})
  qt5_add_resources(exo_RESOURCES_RCC ${exo_RSRCS})
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
add_executable(exo ${exo_SRCS} ${exo_HEADERS_MOC}
                   ${exo_FORMS_HEADERS} ${exo_RESOURCES_RCC})
target_link_libraries(exo ${qt_LIBS} ${LASTFM_LIBRARIES})
target_compile_features(exo PRIVATE cxx_nullptr cxx_generalized_initializers)

install(TARGETS exo RUNTIME DESTINATION bin)
