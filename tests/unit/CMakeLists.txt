include_directories ( ${CMAKE_SOURCE_DIR}/src
					  ${CMAKE_SOURCE_DIR}/deps/rax
					  ${CMAKE_SOURCE_DIR}/deps/xxHash
					  ${CMAKE_SOURCE_DIR}/deps/RediSearch/src
					  ${CMAKE_SOURCE_DIR}/deps/GraphBLAS/Include
					  ${CMAKE_SOURCE_DIR}/deps/libcypher-parser/lib/src )

#-------------------------------------------------------------------------------
# This will only build tests from files with the name "test_*.c"
#-------------------------------------------------------------------------------

file( GLOB TEST_SOURCES LIST_DIRECTORIES false test_*.c )
foreach( testsourcefile ${TEST_SOURCES} )
	get_filename_component( justname ${testsourcefile} NAME )
	string( REPLACE ".c" "" testname ${justname} )
	add_executable( ${testname} ${testsourcefile} )
	target_link_libraries ( ${testname} redisgraph ${OpenMP_C_LIBRARIES} )
	target_link_directories ( ${testname} BEFORE PUBLIC ${CMAKE_SOURCE_DIR}/build )
	string( REPLACE "test_" "ctest_" ctestname ${testname} )
	add_test( ${ctestname} ${testname} )
endforeach( testsourcefile ${TEST_SOURCES} )

