cmake_minimum_required(VERSION 2.8.12)
project(kdevgoplugin)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 90)
set(VERSION_PATCH 90)
# KDevplatform dependency version
set(KDEVPLATFORM_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

find_package(ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(ECMAddTests)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(GenerateExportHeader)

find_package(Qt5 REQUIRED Core Widgets Test)
find_package(KF5 REQUIRED COMPONENTS ItemModels ThreadWeaver TextEditor I18n)
find_package(KDevPlatform ${KDEVPLATFORM_VERSION} REQUIRED)
find_package(KDevelop-PG-Qt REQUIRED)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}
)
    
enable_testing()

add_subdirectory(buildsystem)
add_subdirectory(parser)
add_subdirectory(duchain)
add_subdirectory(codecompletion)

include_directories(
    ${KDEVPGQT_INCLUDE_DIR}
)

kdevplatform_add_plugin(kdevgoplugin JSON kdevgo.json SOURCES
    kdevgoplugin.cpp
    golangparsejob.cpp
    gohighlighting.cpp
    godebug.cpp
)

target_link_libraries(kdevgoplugin 
    KDev::Interfaces
    KDev::Language
    KF5::ThreadWeaver
    KF5::TextEditor
    kdevgoparser
    kdevgoduchain
    kdevgocompletion
)

install(FILES builtins.go DESTINATION ${DATA_INSTALL_DIR}/kdev-go)
