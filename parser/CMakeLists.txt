include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(KDevelop-PG-Qt REQUIRED)

kdevpgqt_generate(go_parser_SRC go NAMESPACE go DEBUG_VISITOR TOKEN_TEXT GENERATE_LEXER
      "${CMAKE_CURRENT_SOURCE_DIR}/go.g")

include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(go_parser_lib_SRC 
    parsesession.cpp
    )

kde4_add_library(kdevgoparser SHARED ${go_parser_SRC} ${go_parser_lib_SRC})

target_link_libraries( kdevgoparser LINK_PRIVATE
   #${KDE4_KDECORE_LIBS}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
)

include_directories(
		${QT_INCLUDES}
		${KDEVPGQT_INCLUDE_DIR}
		)


kde4_add_executable(go_parser main.cpp)

target_link_libraries(go_parser  kdevgoparser
      ${QT_CORE_LIBRARY}
      ${KDE4_KTEXTEDITOR_LIBS}
      )

install(TARGETS kdevgoparser DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(test)
