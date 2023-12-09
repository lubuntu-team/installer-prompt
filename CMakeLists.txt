cmake_minimum_required(VERSION 3.16)

project(lubuntu-installer-prompt VERSION 1.0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INSTALL_PREFIX "/usr")

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core Widgets Network LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Network LinguistTools)
find_package(KF5 REQUIRED COMPONENTS NetworkManagerQt WidgetsAddons)

set(TS_FILES
    src/translations/lubuntu-installer-prompt_en_US.ts
    src/translations/lubuntu-installer-prompt_es_ES.ts
    src/translations/lubuntu-installer-prompt_zh_CN.ts
)

set(PROJECT_SOURCES
    src/main.cpp
    src/installerprompt.cpp
    src/installerprompt.h
    src/installerprompt.ui
    src/resource.qrc
)

set(TRANSLATION_RESOURCES "src/translations.qrc")

configure_file(${TRANSLATION_RESOURCES} translations.qrc COPYONLY)
qt5_add_translation(QM_FILES ${TS_FILES})
qt5_add_resources(QM_RESOURCES ${CMAKE_CURRENT_BINARY_DIR}/translations.qrc)

add_custom_target(translations ALL DEPENDS ${QM_FILES})

add_executable(lubuntu-installer-prompt
    ${PROJECT_SOURCES}
    ${QM_RESOURCES}
)

add_dependencies(lubuntu-installer-prompt translations)

target_link_libraries(lubuntu-installer-prompt PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt5::Network KF5::NetworkManagerQt KF5::WidgetsAddons)

install(TARGETS lubuntu-installer-prompt
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(PROGRAMS "scripts/change-system-language" DESTINATION libexec)
install(PROGRAMS "scripts/lubuntu-installer" DESTINATION libexec)
install(PROGRAMS "scripts/start-lubuntu-live-env" DESTINATION bin)
install(FILES "lubuntu-live-environment.desktop" DESTINATION share/xsessions)
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink /etc/xdg/xdg-Lubuntu /etc/xdg/xdg-lubuntu-live-environment)")

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(lubuntu-installer-prompt)
endif()
