# define test command
set ( baseCommand "RLTEST_ARGS='${RLTEST_ARGS}' ${CMAKE_CURRENT_SOURCE_DIR}/tests.sh libredisgraph.a" )

#-------------------------------------------------------------------------------
# This will only build tests from files with the name "test_*.py"
#-------------------------------------------------------------------------------

file( GLOB PY_TEST_SOURCES LIST_DIRECTORIES false test_*.py )
foreach( testsourcefile ${PY_TEST_SOURCES} )
	get_filename_component ( justname ${testsourcefile} NAME )
	string ( REPLACE ".py" "" testname ${justname} )
	add_test ( NAME "PY_${justname}"
		COMMAND bash -c "${baseCommand} -t ${testsourcefile}"
		WORKING_DIRECTORY ${PROJECT_BINARY_DIR} )
endforeach( )

