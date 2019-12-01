
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -fsanitize=address")

find_package(Qt5 5.9.1 REQUIRED COMPONENTS Core Network Widgets Xml Test)
set(qt_LIBS Qt5::Core Qt5::Network Qt5::Widgets Qt5::Xml)

set(exo_SRCS main.cpp
    core/mocinterface.cpp
    core/playerinterface.cpp
    core/singleinstance.cpp
    gui/aboutdialog.cpp
    gui/basedialog.cpp
    gui/bookmarkmanager.cpp
    gui/lyricsdialog.cpp
    gui/trayicon.cpp)

if(USE_CMUS)
  list(APPEND exo_SRCS core/cmusinterface.cpp)
endif()

if(BUILD_DBUS)
  list(APPEND exo_SRCS dbus/dbus.cpp
      dbus/exoobject.cpp
      dbus/rootobject.cpp
      dbus/playerobject.cpp)
  find_package(Qt5DBus)
  list(APPEND qt_LIBS Qt5::DBus)
endif()

if(BUILD_LASTFM)
  list(APPEND exo_SRCS lastfm/scrobbler.cpp
      lastfm/scrobblerauth.cpp
      core/consoleauth.cpp
      gui/scrobblersettings.cpp)
  find_library(LASTFM_LIBRARIES lastfm5)
endif()

qt5_add_resources(exo_RESOURCES_RCC ../images/exo.qrc)

# translations
if(BUILD_TRANSLATIONS)
  SET(TRANSLATION
      ../translations/be_BY.ts
      ../translations/ru_RU.ts
  )
  find_package(Qt5LinguistTools REQUIRED)
  qt5_add_translation(QM_FILES ${TRANSLATION})
endif()

add_executable(${PROJECT_NAME} ${exo_SRCS} ${exo_HEADERS_MOC} ${exo_RESOURCES_RCC} ${QM_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR} core dbus gui lastfm)
target_link_libraries(${PROJECT_NAME} PRIVATE ${qt_LIBS} ${LASTFM_LIBRARIES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_nullptr cxx_generalized_initializers)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

if(BUILD_TRANSLATIONS)
  install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/exo/translations)
endif()

# tests
if(BUILD_TESTS)
  set(test_LIBS Qt5::Core Qt5::Test)
  set(tests_SRCS tests/tests.cpp
      tests/pitests.cpp
      core/playerinterface.cpp
      core/mocinterface.cpp
      core/cmusinterface.cpp)
  add_executable(tests ${tests_SRCS})
  target_include_directories(tests PRIVATE ${CMAKE_CURRENT_BINARY_DIR} core)
  target_link_libraries(tests PRIVATE ${test_LIBS})
  target_compile_features(tests PRIVATE cxx_nullptr)
endif()
