ecm_add_test(testduchain.cpp
        TEST_NAME duchain
        LINK_LIBRARIES
        Qt5::Test
        KDev::Language
        KDev::Tests
        kdevgoduchain
        kdevgoparser
)
