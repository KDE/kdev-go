ecm_add_test(testcompletion.cpp
        TEST_NAME completion
        LINK_LIBRARIES
        Qt5::Test
        KDev::Language
        KDev::Tests
        kdevgoduchain
        kdevgoparser
        kdevgocompletion
)
