
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(CMAKE_AUTOMOC ON)

if(WITH_QT6)
    find_package(Qt6 6.2 REQUIRED COMPONENTS Core Network Widgets Xml DBus Test)
    set(qt_LIBS Qt6::Core Qt6::Network Qt6::Widgets Qt6::Xml Qt6::DBus)
else()
    find_package(Qt5 5.15.0 REQUIRED COMPONENTS Core Network Widgets Xml DBus Test)
    set(qt_LIBS Qt5::Core Qt5::Network Qt5::Widgets Qt5::Xml Qt5::DBus)
endif()

set(exo_SRCS main.cpp
    core/cmusinterface.cpp
    core/mocinterfacenative.cpp
    core/mocinterface.cpp
    core/playerinterface.cpp
    core/singleinstance.cpp
    core/spotifyinterface.cpp
    core/sysutils.cpp
    dbus/dbus.cpp
    dbus/playerobject.cpp
    dbus/rootobject.cpp
    gui/aboutdialog.cpp
    gui/basedialog.cpp
    gui/bookmarkmanager.cpp
    gui/lyricsproviders.cpp
    gui/lyricsdialog.cpp
    gui/trayicon.cpp
)

if(BUILD_LASTFM)
  list(APPEND exo_SRCS lastfm/scrobbler.cpp
      lastfm/scrobblerauth.cpp
      core/consoleauth.cpp
      gui/scrobblersettings.cpp)
  if(WITH_QT6)
    # NOTE: lastfm5 not supposed to be installed, otherwise may link against it
    # as a workaround give full path to the library:
    #   -DLASTFM_LIBRARIES=/usr/lib/x86_64-linux-gnu/liblastfm6.so
    find_library(LASTFM_LIBRARIES lastfm6)
  else()
    find_library(LASTFM_LIBRARIES lastfm5)
  endif()
endif()

if(WITH_QT6)
  qt_add_resources(exo_RESOURCES_RCC ../images/exo.qrc)
else()
  qt5_add_resources(exo_RESOURCES_RCC ../images/exo.qrc)
endif()

# translations
SET(TRANSLATION
    ../translations/be_BY.ts
    ../translations/pl_PL.ts
    ../translations/pt_BR.ts
    ../translations/ru_RU.ts
    )
if(WITH_QT6)
  find_package(Qt6LinguistTools REQUIRED)
  qt_add_translation(QM_FILES ${TRANSLATION})
else()
  find_package(Qt5LinguistTools REQUIRED)
  qt5_add_translation(QM_FILES ${TRANSLATION})
endif()

add_executable(exo ${exo_SRCS} ${exo_HEADERS_MOC} ${exo_RESOURCES_RCC} ${QM_FILES})
target_include_directories(exo PRIVATE ${CMAKE_CURRENT_BINARY_DIR} core dbus gui lastfm)
target_link_libraries(exo PRIVATE ${qt_LIBS} ${LASTFM_LIBRARIES})
target_compile_features(exo PRIVATE cxx_std_17)

install(TARGETS exo RUNTIME DESTINATION bin)
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/exo/translations)

# tests
if(BUILD_TESTS)
  if(WITH_QT6)
    set(test_LIBS Qt6::Core Qt6::Test)
  else()
    set(test_LIBS Qt5::Core Qt5::Test)
  endif()
  set(tests_SRCS tests/tests.cpp
      tests/pitests.cpp
      tests/systests.cpp
      core/sysutils.cpp
      core/playerinterface.cpp
      core/mocinterface.cpp
      core/cmusinterface.cpp)

  set(BUILD_LASTFM OFF)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
                 ${CMAKE_CURRENT_BINARY_DIR}/tests_includedir/config.h)

  add_executable(tests ${tests_SRCS})
  target_include_directories(tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/tests_includedir core)
  target_link_libraries(tests PRIVATE ${test_LIBS})
  target_compile_features(tests PRIVATE cxx_std_17)
endif()
