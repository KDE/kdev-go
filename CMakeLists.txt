cmake_minimum_required(VERSION 2.8)
project(kdevgoplugin)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
                      ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
)

find_package(KDE4 REQUIRED)
find_package(KDevPlatform REQUIRED)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

include_directories(
    ${KDE4_INCLUDES}
    ${KDEVPLATFORM_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/parser
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
    )
    
kde4_add_plugin(kdevgoplugin ${goplugin_SRC})
target_link_libraries(kdevgoplugin 
	  ${KDEVPLATFORM_INTERFACES_LIBRARIES} 
	  ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
	  ${KDE4_THREADWEAVER_LIBS}
	  kdevgoparser
	  kdevgoduchain
	  kdevgocompletion
	  )

install(TARGETS kdevgoplugin DESTINATION ${PLUGIN_INSTALL_DIR})

configure_file(kdev-go.desktop.in ${CMAKE_BINARY_DIR}/kdev-go.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdev-go.desktop DESTINATION ${SERVICES_INSTALL_DIR})
