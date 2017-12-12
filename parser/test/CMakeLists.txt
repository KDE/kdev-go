ecm_add_test(parsertest.cpp
        LINK_LIBRARIES
        kdevgoparser
        Qt5::Test
        Qt5::Core
        KDev::Language
        KDev::Tests
        )