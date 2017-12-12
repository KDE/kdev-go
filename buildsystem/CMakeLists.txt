set(buildsystem_PART_SRCS
    buildsystem.cpp
    builder.cpp
    buildjob.cpp
    executabletargetitem.cpp
    utils.cpp
    builddirchooser.cpp
    preferences.cpp
)

ki18n_wrap_ui(buildsystem_PART_SRCS builddirchooser.ui)

qt5_add_resources(buildsystem_PART_SRCS buildsystem.qrc)
kdevplatform_add_plugin(gobuildsystem JSON buildsystem.json SOURCES ${buildsystem_PART_SRCS})
generate_export_header(gobuildsystem BASE_NAME gobuildsystem EXPORT_MACRO_NAME KDEVGOBUILDSYSTEM_EXPORT)
target_link_libraries(gobuildsystem
    KF5::KIOWidgets
    KDev::Interfaces KDev::Project KDev::Util KDev::Language KDev::OutputView KDev::Shell
)