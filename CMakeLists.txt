cmake_minimum_required(VERSION 2.8)
project(kdevgoplugin)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 90)
set(VERSION_PATCH 90)
# KDevplatform dependency version
set( KDEVPLATFORM_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" )

find_package (ECM 0.0.9 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(ECMAddTests)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(Qt5 REQUIRED Core Widgets Test)
find_package(KF5 REQUIRED COMPONENTS ItemModels ThreadWeaver TextEditor I18n)
find_package(KDevPlatform ${KDEVPLATFORM_VERSION} REQUIRED)
find_package(KDevelop-PG-Qt REQUIRED)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}
    )
    
enable_testing()

add_subdirectory(parser)
add_subdirectory(duchain)
add_subdirectory(codecompletion)

include_directories(
    ${KDEVPGQT_INCLUDE_DIR}
    )



set(goplugin_SRC 
    kdevgoplugin.cpp
    golangparsejob.cpp
    gohighlighting.cpp
    godebug.cpp
    )
    
add_library(kdevgoplugin MODULE ${goplugin_SRC})
target_link_libraries(kdevgoplugin 
        KDev::Interfaces
        KDev::Language
        KF5::ThreadWeaver
        KF5::TextEditor
        kdevgoparser
        kdevgoduchain
        kdevgocompletion
        )

install(TARGETS kdevgoplugin DESTINATION ${PLUGIN_INSTALL_DIR})

configure_file(kdev-go.desktop.in ${CMAKE_BINARY_DIR}/kdev-go.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdev-go.desktop DESTINATION ${SERVICES_INSTALL_DIR})
