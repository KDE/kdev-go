include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

########### next target ###############

kde4_add_unit_test(testcompletion testcompletion.cpp)
target_link_libraries(testcompletion
    ${KDE4_KDECORE_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    kdevgoduchain
    kdevgoparser
    kdevgocompletion
)
