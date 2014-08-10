include_directories(${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

    
include_directories(
		${QT_INCLUDES}
		 ${KDEVPGQT_INCLUDE_DIR}
		) 
		 
set(goparsertest_SRC parsertest.cpp)

kde4_add_unit_test(goparsertest ${goparsertest_SRC})

target_link_libraries(goparsertest 
		kdevgoparser
		${QT_QTTEST_LIBRARY} 
		${QT_QTCORE_LIBRARY} 
		${KDE4_KDECORE_LIBS} 
		#${KDEVPLATFORM_LANGUAGE_LIBRARIES}
		)