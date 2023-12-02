cmake_minimum_required(VERSION 3.22)

project(lubuntu-installer-prompt VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB PROJECT_SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.ui"
)

add_executable(lubuntu-installer-prompt ${PROJECT_SOURCES})

target_link_libraries(lubuntu-installer-prompt Qt5::Widgets)

install(TARGETS lubuntu-installer-prompt DESTINATION bin)
install(PROGRAMS "scripts/lubuntu-installer" DESTINATION libexec)
install(PROGRAMS "scripts/start-lubuntu-live-env" DESTINATION bin)
install(FILES "img/background.png" DESTINATION share/lubuntu/installer-prompt)
install(FILES "lubuntu-live-environment.desktop" DESTINATION share/xsessions)
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink /etc/xdg/xdg-Lubuntu /etc/xdg/xdg-lubuntu-live-environment)")
