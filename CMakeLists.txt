cmake_minimum_required(VERSION 3.13)

get_filename_component(root ${CMAKE_CURRENT_LIST_DIR} ABSOLUTE)
include(${root}/deps/readies/cmake/main)

if (NOT DEFINED UNIT_TESTS)
	set(UNIT_TESTS OFF)
endif()

project(redisgraph)

file(GLOB_RECURSE SOURCES "src/*.c")

if (NOT APPLE)
	add_library(redisgraph SHARED ${SOURCES})
else()
	add_library(redisgraph MODULE ${SOURCES})
endif()

set_property(TARGET redisgraph PROPERTY C_STANDARD 11)

include_directories(
	${root}/src
	${root}
	${root}/deps
	${root}/deps/rax
	${root}/deps/xxHash
	${root}/deps/utf8proc
	${root}/deps/RediSearch/src
	${root}/deps/GraphBLAS/Include
	${root}/deps/libcypher-parser/lib/src
	$ENV{LIBCYPHER_PARSER_BINDIR}/lib/src)

add_compile_definitions(
	_GNU_SOURCE
	REDIS_MODULE_TARGET
	REDISMODULE_EXPERIMENTAL_API
	XXH_STATIC_LINKING_ONLY)

if (DEBUG)
	add_compile_definitions(RG_DEBUG)
endif()

add_compile_options(${CMAKE_CC_FLAGS} ${CMAKE_CC_C_FLAGS})
target_compile_options(redisgraph PRIVATE -fPIC ${CMAKE_CC_FLAGS} ${CMAKE_CC_C_FLAGS})
if (MEMCHECK)
	add_compile_definitions(MEMCHECK)
endif()

lists_from_env(LIBOMP)
target_link_options(redisgraph PRIVATE ${CMAKE_LD_FLAGS} ${CMAKE_SO_LD_FLAGS} ${LIBOMP})

set_target_properties(redisgraph PROPERTIES PREFIX "")
set_target_properties(redisgraph PROPERTIES SUFFIX ".so")

set_target_properties(redisgraph PROPERTIES LINKER_LANGUAGE CXX)

set(REDISGRAPH_OBJECTS $<TARGET_OBJECTS:redisgraph>)

lists_from_env(GRAPHBLAS LIBXXHASH RAX LIBCYPHER_PARSER REDISEARCH_LIBS UTF8PROC)
set(REDISGRAPH_LIBS ${GRAPHBLAS} ${LIBXXHASH} ${RAX} ${LIBCYPHER_PARSER} ${REDISEARCH_LIBS} ${UTF8PROC})

target_link_libraries(redisgraph PRIVATE ${REDISGRAPH_LIBS} c m dl pthread)

extract_debug_symbols(redisgraph)

if (APPLE)
	# add_library(redisgraph_dylib SHARED $<TARGET_OBJECTS:redisgraph>)
	# target_link_libraries(redisgraph_dylib PRIVATE ${REDISGRAPH_LIBS} ${LIBOMP} c m dl pthread)
	
	add_library(redisgraph_static STATIC $<TARGET_OBJECTS:redisgraph>)
endif()

if (UNIT_TESTS)
	add_subdirectory(${root}/tests/unit tests/unit)
endif()
