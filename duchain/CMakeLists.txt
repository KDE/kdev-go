
set(duchain_SRC
     builders/declarationbuilder.cpp
     builders/contextbuilder.cpp
     builders/typebuilder.cpp
     builders/usebuilder.cpp
     goducontext.cpp
     expressionvisitor.cpp
     helper.cpp
     duchaindebug.cpp
     
     types/gointegraltype.cpp
     types/gofunctiontype.cpp
     types/gostructuretype.cpp
     types/gomaptype.cpp
     types/gochantype.cpp
     declarations/functiondeclaration.cpp
     declarations/functiondefinition.cpp

     navigation/navigationwidget.cpp
     navigation/declarationnavigationcontext.cpp
     )
     
add_library( kdevgoduchain SHARED ${duchain_SRC})
generate_export_header(kdevgoduchain BASE_NAME kdevgoduchain EXPORT_MACRO_NAME KDEVGODUCHAIN_EXPORT)
target_link_libraries(kdevgoduchain LINK_PRIVATE
    KDev::Interfaces
    KDev::Language
    KDev::Shell
    KDev::Project
    kdevgoparser
 )

install(TARGETS kdevgoduchain DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(tests)