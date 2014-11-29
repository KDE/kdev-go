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
    items/completionitem.cpp
    items/functionitem.cpp
    items/importcompletionitem.cpp
)

kde4_add_library(kdevgocompletion SHARED ${completion_SRCS})

target_link_libraries(kdevgocompletion LINK_PRIVATE
    ${KDE4_KDECORE_LIBS}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_INTERFACES_LIBRARIES}
    ${KDEVPLATFORM_PROJECT_LIBRARIES}
    kdevgoduchain
    kdevgoparser
)

install(TARGETS kdevgocompletion DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(tests)
