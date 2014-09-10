include_directories(${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

include_directories(
		${QT_INCLUDES}
		 ${KDEVPGQT_INCLUDE_DIR}
		) 
		 
ecm_add_test(parsertest.cpp
        LINK_LIBRARIES
        kdevgoparser
        Qt5::Test
        Qt5::Core
        KDev::Language
        )