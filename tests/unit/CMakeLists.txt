
set(BUILD_GTEST ON CACHE BOOL "enable gtest" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "disable gmock" FORCE)

add_subdirectory(${root}/deps/googletest gtest)

include_directories(${root}/deps/googletest/googletest/include)
include_directories(${root}/deps/googletest/googletest/include/gtest)

file(GLOB TEST_SOURCES LIST_DIRECTORIES false test_*.cpp)

add_executable(unit-tests ${TEST_SOURCES} main.cpp)
target_link_libraries(unit-tests gtest redisgraph ${REDISGRAPH_LIBS} ${CMAKE_LD_LIBS})
set_target_properties(unit-tests PROPERTIES LINKER_LANGUAGE CXX)

#foreach(test_src ${TEST_SOURCES})
#	get_filename_component(test ${test_src} NAME_WE)
#	add_executable(${test} ${test_src})
#	set_target_properties(${test} PROPERTIES LINKER_LANGUAGE CXX)
#	if (NOT APPLE)
#		target_link_libraries(${test} PRIVATE redisgraph ${REDISGRAPH_LIBS} ${CMAKE_LD_LIBS} googletest)
#	else()
#		target_link_libraries(${test} PRIVATE ${REDISGRAPH_OBJECTS} ${REDISGRAPH_LIBS} ${CMAKE_LD_LIBS} googletest)
#	endif()
#endforeach()
