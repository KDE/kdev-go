add_definitions(-DTRANSLATION_DOMAIN=\"buildsystem\")

set(buildsystem_PART_SRCS
    buildsystem.cpp
    builder.cpp
    buildjob.cpp
)

qt5_add_resources(buildsystem_PART_SRCS buildsystem.qrc)
kdevplatform_add_plugin(gobuildsystem JSON buildsystem.json SOURCES ${buildsystem_PART_SRCS})
target_link_libraries(gobuildsystem
    KF5::KIOWidgets
    KDev::Interfaces KDev::Project KDev::Util KDev::Language KDev::OutputView
)
