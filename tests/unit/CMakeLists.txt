
file(GLOB TEST_SOURCES LIST_DIRECTORIES false test_*.c)

foreach(test_src ${TEST_SOURCES})
	get_filename_component(test ${test_src} NAME_WE)
	add_executable(${test} ${test_src})
	set_target_properties(${test} PROPERTIES LINKER_LANGUAGE CXX)
	if (NOT APPLE)
		target_link_libraries(${test} PRIVATE redisgraph ${REDISGRAPH_LIBS} ${CMAKE_LD_LIBS})
	else()
		target_link_libraries(${test} PRIVATE ${REDISGRAPH_OBJECTS} ${REDISGRAPH_LIBS} ${CMAKE_LD_LIBS})
	endif()
endforeach()
