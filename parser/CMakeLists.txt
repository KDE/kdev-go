cmake_minimum_required(VERSION 3.6)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(KDevelop-PG-Qt REQUIRED)

find_package(ECM REQUIRED)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddTests)
include(KDEInstallDirs)

find_package(KDevPlatform REQUIRED)

kdevpgqt_generate(go_parser_SRC go NAMESPACE go DEBUG_VISITOR TOKEN_TEXT GENERATE_LEXER
      "${CMAKE_CURRENT_SOURCE_DIR}/go.g")

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(go_parser_lib_SRC 
    parsesession.cpp
)

add_library(kdevgoparser SHARED ${go_parser_SRC} ${go_parser_lib_SRC})
include(GenerateExportHeader)
generate_export_header(kdevgoparser BASE_NAME kdevgoparser EXPORT_MACRO_NAME KDEVGOPARSER_EXPORT)
target_link_libraries( kdevgoparser LINK_PRIVATE
    KDev::Language
)

include_directories(
		${QT_INCLUDES}
		${KDEVPGQT_INCLUDE_DIR}
		)


add_executable(go_parser main.cpp)

target_link_libraries(go_parser  kdevgoparser
        KDev::Language
      )

install(TARGETS kdevgoparser DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(test)

include(FeatureSummary)
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

