include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/duchain
    ${KDEVPGQT_INCLUDE_DIR}
)

set(completion_SRCS
    model.cpp
    worker.cpp
    context.cpp
    completiondebug.cpp
)

add_library(kdevgocompletion SHARED ${completion_SRCS})

target_link_libraries(kdevgocompletion LINK_PRIVATE
    KDev::Language
    KDev::Interfaces
    KDev::Project
    kdevgoduchain
    kdevgoparser
)

install(TARGETS kdevgocompletion DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})