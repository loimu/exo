project(exo)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(exo_SRCS exo.cpp main.cpp)
qt4_automoc(${exo_SRCS})
add_executable(exo ${exo_SRCS})
target_link_libraries(exo ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})
install(TARGETS exo RUNTIME DESTINATION bin)
