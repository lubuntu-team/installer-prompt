cmake_minimum_required(VERSION 3.22)

project(lubuntu-installer-prompt VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

include_directories(${PROJECT_SOURCE_DIR}/src)

file(GLOB PROJECT_SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/*.h"
    "${PROJECT_SOURCE_DIR}/src/*.ui"
)

qt_add_executable(lubuntu-installer-prompt
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(lubuntu-installer-prompt PRIVATE Qt6::Widgets)

qt_finalize_executable(lubuntu-installer-prompt)

install(TARGETS lubuntu-installer-prompt DESTINATION bin)
install(PROGRAMS "scripts/lubuntu-installer" DESTINATION bin)
install(FILES "img/background.png" DESTINATION share/lubuntu/installer-prompt)
