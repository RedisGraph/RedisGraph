cmake_minimum_required ( VERSION 3.2 )

set ( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake_modules )

include ( ExternalProject )
#cmake_policy ( SET CMP0114 NEW )

# version of RedisGraph
set ( RedisGraph_DATE "Sept 20, 2022" )
set ( RedisGraph_VERSION_MAJOR 1 )
set ( RedisGraph_VERSION_MINOR 0 )
set ( RedisGraph_VERSION_SUB   0 )

project ( redisgraph
	VERSION "${RedisGraph_VERSION_MAJOR}.${RedisGraph_VERSION_MINOR}.${RedisGraph_VERSION_SUB}"
	LANGUAGES C )

#-------------------------------------------------------------------------------
# code coverage and build type
#-------------------------------------------------------------------------------

# to compile with test coverage:
#   cd build
#   cmake -DCOVERAGE=1 ..
#   make -j8
#   make test_coverage

if ( "${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
	if ( COVERAGE )
		message ( STATUS "============== Code coverage enabled ===============" )
		set ( CMAKE_BUILD_TYPE Debug )
		# On the Mac, you need gcov-11 from homebrew (part of gcc-11):
		# and uncomment this line:
		set ( GCOV_PATH /usr/local/bin/gcov-11)
		include ( CodeCoverage )

		append_coverage_compiler_flags ( )

		# turn off optimization for non-skewed coverage reports
		set ( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -DCOVERAGE" )
		set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0" )

		setup_target_for_coverage_lcov (
			NAME test_coverage
			EXECUTABLE ctest
			DEPENDENCIES ${PROJECT_NAME}
			BASE_DIRECTORY "."
			NO_DEMANGLE TRUE
			)
	endif ( )
endif ( )

# For development only, not for end-users:
set ( CMAKE_BUILD_TYPE Debug )

if ( NOT CMAKE_BUILD_TYPE )
	set ( CMAKE_BUILD_TYPE Release )
endif ( )

find_package ( OpenMP REQUIRED )

#-------------------------------------------------------------------------------
# report status
#-------------------------------------------------------------------------------

#message ( STATUS "cmake_lang_compiler_id: " ${CMAKE_C_COMPILER_ID} )
#message ( STATUS "cmake_lang_compiler_v: " ${CMAKE_C_COMPILER_VERSION} )
#message ( STATUS "cmake_compiler_is_gnucxx " CMAKE_COMPILER_IS_GNUCXX )

message ( STATUS "CMAKE build type:          " ${CMAKE_BUILD_TYPE} )
message ( STATUS "CMAKE source directory:    " ${CMAKE_SOURCE_DIR} )
message ( STATUS "CMAKE build directory:     " ${CMAKE_BINARY_DIR} )
message ( STATUS "CMAKE compiler ID:         " ${CMAKE_C_COMPILER_ID} )

if ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	message ( STATUS "CMAKE C Flags debug:       " ${CMAKE_C_FLAGS_DEBUG} )
else ( )
	message ( STATUS "CMAKE C Flags release:     " ${CMAKE_C_FLAGS_RELEASE} )
endif ( )

#-------------------------------------------------------------------------------
# compiler options
#-------------------------------------------------------------------------------

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

if ( ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}" )
else ( )
	set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}" )
endif ( )

#-------------------------------------------------------------------------------
# print final C flags
#-------------------------------------------------------------------------------

message ( STATUS "CMAKE C flags: " ${CMAKE_C_FLAGS} )

#-------------------------------------------------------------------------------
# enable testing and add subdirectories
#-------------------------------------------------------------------------------

enable_testing ( )

add_subdirectory ( src )
add_subdirectory ( tests/unit )
add_subdirectory ( tests/flow )

#-------------------------------------------------------------------------------
# xxHash
#-------------------------------------------------------------------------------

ExternalProject_Add ( xxHash
	BUILD_IN_SOURCE TRUE
	PREFIX ${CMAKE_SOURCE_DIR}/deps/xxHash
	SOURCE_DIR ${CMAKE_SOURCE_DIR}/deps/xxHash 
	CONFIGURE_COMMAND ""
	BUILD_COMMAND $(MAKE) lib
	INSTALL_COMMAND "" )

ExternalProject_Add_StepTargets(xxHash build)

#-------------------------------------------------------------------------------
# GraphBLAS
#-------------------------------------------------------------------------------

ExternalProject_Add ( GraphBLAS
	PREFIX ${CMAKE_SOURCE_DIR}/deps/GraphBLAS
	BUILD_IN_SOURCE TRUE
	SOURCE_DIR ${CMAKE_SOURCE_DIR}/deps/GraphBLAS
	CONFIGURE_COMMAND cd build && cmake $(CMAKE_OPTIONS) -DBUILD_GRB_STATIC_LIBRARY=1 ..
	BUILD_COMMAND cd build && $(MAKE)
	INSTALL_COMMAND "" )

ExternalProject_Add_StepTargets(GraphBLAS build)

#-------------------------------------------------------------------------------
# libcypher-parser
#-------------------------------------------------------------------------------

ExternalProject_Add ( cypher-parser
	BUILD_IN_SOURCE TRUE
	PREFIX ${CMAKE_SOURCE_DIR}/deps/libcypher-parser
	SOURCE_DIR ${CMAKE_SOURCE_DIR}/deps/libcypher-parser
	CONFIGURE_COMMAND ./autogen.sh && ./configure --disable-shared
	BUILD_COMMAND $(MAKE)
	INSTALL_COMMAND "" )

ExternalProject_Add_StepTargets(cypher-parser build)

#-------------------------------------------------------------------------------
# RediSearch
#-------------------------------------------------------------------------------

ExternalProject_Add ( redisearch
	BUILD_IN_SOURCE TRUE
	PREFIX ${CMAKE_SOURCE_DIR}/deps/RediSearch
	SOURCE_DIR ${CMAKE_SOURCE_DIR}/deps/RediSearch
	CONFIGURE_COMMAND ""
	BUILD_COMMAND $(MAKE) STATIC=1
	INSTALL_COMMAND "" )

ExternalProject_Add_StepTargets(redisearch build)

#-------------------------------------------------------------------------------
# rax
#-------------------------------------------------------------------------------

ExternalProject_Add ( rax
	BUILD_IN_SOURCE TRUE
	PREFIX ${CMAKE_SOURCE_DIR}/deps/rax
	SOURCE_DIR ${CMAKE_SOURCE_DIR}/deps/rax
	CONFIGURE_COMMAND ""
	BUILD_COMMAND $(MAKE)
	INSTALL_COMMAND "" )

ExternalProject_Add_StepTargets(rax build)

