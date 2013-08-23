project(exo)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(exo_SRCS src/exo.cpp src/main.cpp
             src/aboutdialog.cpp
             src/lyricswindow.cpp
             src/playerinterface.cpp
             src/trayicon.cpp)
set(exo_HDRS src/aboutdialog.h src/exo.h
             src/lyricswindow.h
             src/playerinterface.h
             src/trayicon.h)
set(exo_FRMS src/aboutdialog.ui src/lyricswindow.ui)
set(exo_RSRCS exo.qrc)
qt4_automoc(${exo_SRCS})
qt4_wrap_cpp(exo_HEADERS_MOC ${exo_HDRS})
qt4_wrap_ui(exo_FORMS_HEADERS ${exo_FRMS})
qt4_add_resources(exo_RESOURCES_RCC ${exo_RSRCS})
add_executable(exo ${exo_SRCS}
                   ${exo_HEADERS_MOC}
                   ${exo_FORMS_HEADERS}
                   ${exo_RESOURCES_RCC})
target_link_libraries(exo ${QT_QTCORE_LIBRARY}
                          ${QT_QTGUI_LIBRARY}
                          ${QT_QTNETWORK_LIBRARY})
install(TARGETS exo RUNTIME DESTINATION bin)
