include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

########### next target ###############

set(testduchain_SRC testduchain.cpp)

kde4_add_unit_test(testduchain ${testduchain_SRC})
target_link_libraries(testduchain
    ${KDE4_KDECORE_LIBS}
    ${QT_QTTEST_LIBRARY}
    ${KDEVPLATFORM_LANGUAGE_LIBRARIES}
    ${KDEVPLATFORM_TESTS_LIBRARIES}
    kdevgoduchain
    kdevgoparser
)
