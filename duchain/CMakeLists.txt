
include_directories(${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${KDEVPGQT_INCLUDE_DIR}
    )
    
set(duchain_SRC
     declarationbuilder.cpp
     contextbuilder.cpp
     usebuilder.cpp
     goducontext.cpp
     expressionvisitor.cpp
     helper.cpp
     
     types/gointegraltype.cpp
     types/gofunctiontype.cpp
     types/gostructuretype.cpp
     types/gomaptype.cpp
     types/gochantype.cpp
     declarations/functiondeclaration.cpp
     
     navigation/navigationwidget.cpp
     navigation/declarationnavigationcontext.cpp
     )
     
kde4_add_library( kdevgoduchain SHARED ${duchain_SRC})

target_link_libraries(kdevgoduchain LINK_PRIVATE
    ${KDE4_KDECORE_LIBS}
    #${KDE4_KTEXTEDITOR_LIBS}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_SHELL_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    kdevgoparser
 )

install(TARGETS kdevgoduchain DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(tests)