cmake_minimum_required(VERSION 3.0)
project(kdevgoplugin)

find_package (ECM "5.15.0" REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(GenerateExportHeader)
include(FeatureSummary)

set(QT_MIN_VERSION "5.5.0")
find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED Core Widgets Test)
set(KF5_DEP_VERSION "5.15.0")
find_package(KF5 ${KF5_DEP_VERSION} REQUIRED COMPONENTS
    ThreadWeaver
    TextEditor
    I18n
)
set(KDEVPLATFORM_DEP_VERSION "5.2.40")
find_package(KDevPlatform ${KDEVPLATFORM_DEP_VERSION} CONFIG)
set_package_properties(KDevPlatform PROPERTIES
    TYPE REQUIRED
)
find_package(KDevelop-PG-Qt CONFIG)
set_package_properties(KDevelop-PG-Qt PROPERTIES
    TYPE REQUIRED
)

add_definitions(-DTRANSLATION_DOMAIN=\"kdevgo\")

# needed for parser/ module prefix with includes for some minimal namespacing
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

add_subdirectory(app_templates)
add_subdirectory(buildsystem)
add_subdirectory(parser)
add_subdirectory(duchain)
add_subdirectory(codecompletion)
add_subdirectory(gometalinter)

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

install(FILES builtins.go DESTINATION ${KDE_INSTALL_DATADIR}/kdev-go)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
