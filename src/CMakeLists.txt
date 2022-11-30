cmake_minimum_required(VERSION 3.13)

get_filename_component(root ${CMAKE_CURRENT_LIST_DIR}/.. ABSOLUTE)
include(${root}/deps/readies/cmake/main)

project(redisgraph)

file(GLOB_RECURSE SOURCES "*.c")

add_library(redisgraph SHARED ${SOURCES})

set_property(TARGET redisgraph PROPERTY C_STANDARD 11)

include_directories(
	${root}/src
	${root}
	${root}/deps
	${root}/deps/rax
	${root}/deps/xxHash
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
	add_compile_definitions(_DEBUG RG_DEBUG)
endif()

lists_from_env(CMAKE_CC_FLAGS CMAKE_CC_C_FLAGS)
add_compile_options(${CMAKE_CC_FLAGS} ${CMAKE_CC_C_FLAGS})
target_compile_options(redisgraph PUBLIC -fPIC ${CMAKE_CC_FLAGS} ${CMAKE_CC_C_FLAGS})

lists_from_env(CMAKE_LD_FLAGS CMAKE_SO_LD_FLAGS LIBOMP)
add_link_options(${CMAKE_LD_FLAGS} ${CMAKE_SO_LD_FLAGS} -Wl,-Bsymbolic,-Bsymbolic-functions ${LIBOMP})
target_link_options(redisgraph PUBLIC ${CMAKE_LD_FLAGS} ${CMAKE_SO_LD_FLAGS} -Wl,-Bsymbolic,-Bsymbolic-functions ${LIBOMP})

set_target_properties(redisgraph PROPERTIES PREFIX "")
set_target_properties(redisgraph PROPERTIES SUFFIX ".so")

set_target_properties(redisgraph PROPERTIES LINKER_LANGUAGE CXX)

set(REDISGRAPH_OBJECTS $<TARGET_OBJECTS:redisgraph>)

lists_from_env(GRAPHBLAS LIBXXHASH RAX LIBCYPHER_PARSER REDISEARCH_LIBS)
set(REDISGRAPH_LIBS ${GRAPHBLAS} ${LIBXXHASH} ${RAX} ${LIBCYPHER_PARSER} ${REDISEARCH_LIBS})

target_link_libraries(redisgraph PRIVATE ${REDISGRAPH_LIBS} c m dl pthread)
#target_link_libraries(redisgraph PRIVATE c m dl pthread)

add_library(redisgraph_static STATIC $<TARGET_OBJECTS:redisgraph>)

add_subdirectory(${root}/tests/unit tests/unit)
