#-------------------------------------------------------------------------------
# include directories
#-------------------------------------------------------------------------------

include_directories ( ${CMAKE_SOURCE_DIR}/src
					  ${CMAKE_SOURCE_DIR}/deps/rax
					  ${CMAKE_SOURCE_DIR}/deps/xxHash
					  ${CMAKE_SOURCE_DIR}/deps/RediSearch/src
					  ${CMAKE_SOURCE_DIR}/deps/GraphBLAS/Include
					  ${CMAKE_SOURCE_DIR}/deps/libcypher-parser/lib/src )

# xxhash: gives access to internal state declaration
# required for static allocation.
# Incompatible with dynamic linking, due to risks of ABI changes.
add_compile_definitions(XXH_STATIC_LINKING_ONLY)

#-------------------------------------------------------------------------------
# sources
#-------------------------------------------------------------------------------

file ( GLOB_RECURSE REDISGRAPH_SOURCES "*.c" )

#-------------------------------------------------------------------------------
# dynamic redisgraph library properties
#-------------------------------------------------------------------------------

add_library ( redisgraph SHARED ${REDISGRAPH_SOURCES} )

#-------------------------------------------------------------------------------
# add OpenMP
#-------------------------------------------------------------------------------

target_link_libraries ( redisgraph PUBLIC ${OpenMP_C_LIBRARIES} )
set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} " )
include_directories ( ${OpenMP_C_INCLUDE_DIRS} )

#-------------------------------------------------------------------------------
# define dependencies
#-------------------------------------------------------------------------------

add_dependencies ( redisgraph rax-build)
add_dependencies ( redisgraph xxHash-build )
add_dependencies ( redisgraph GraphBLAS-build )
add_dependencies ( redisgraph redisearch-build)
add_dependencies ( redisgraph cypher-parser-build )

set_target_properties ( redisgraph PROPERTIES
	VERSION ${RedisGraph_VERSION_MAJOR}.${RedisGraph_VERSION_MINOR}.${RedisGraph_VERSION_SUB}
	LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build
	SOVERSION ${RedisGraph_VERSION_MAJOR}
	C_STANDARD_REQUIRED 11 )
set_property ( TARGET redisgraph PROPERTY C_STANDARD 11 )

# rax
target_link_libraries( redisgraph PRIVATE ${CMAKE_SOURCE_DIR}/deps/rax/rax.o )
# xxhash
target_link_libraries( redisgraph PRIVATE ${CMAKE_SOURCE_DIR}/deps/xxHash/libxxhash.a )
# GraphBLAS
target_link_libraries ( redisgraph PUBLIC ${CMAKE_SOURCE_DIR}/deps/GraphBLAS/build/libgraphblas.a )
# redisearch
#target_link_libraries( redisgraph PRIVATE ${CMAKE_SOURCE_DIR}/bin/macos-arm64v8-release/search-static/libredisearch.a )
target_link_libraries( redisgraph PRIVATE ${CMAKE_SOURCE_DIR}/bin/macos-arm64v8-debug/search-static/libredisearch.a )
# libcypher-parser
target_link_libraries( redisgraph PRIVATE ${CMAKE_SOURCE_DIR}/deps/libcypher-parser/lib/src/.libs/libcypher-parser.a )

#-------------------------------------------------------------------------------
# static redisgraph library properties
#-------------------------------------------------------------------------------

add_library ( redisgraph_static STATIC ${REDISGRAPH_SOURCES} )

#-------------------------------------------------------------------------------
# add OpenMP
#-------------------------------------------------------------------------------

target_link_libraries ( redisgraph_static PUBLIC ${OpenMP_C_LIBRARIES} )
set ( CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} " )
include_directories ( ${OpenMP_C_INCLUDE_DIRS} )

#-------------------------------------------------------------------------------
# define dependencies
#-------------------------------------------------------------------------------

add_dependencies ( redisgraph_static rax-build)
add_dependencies ( redisgraph_static xxHash-build )
add_dependencies ( redisgraph_static GraphBLAS-build )
add_dependencies ( redisgraph_static redisearch-build)
add_dependencies ( redisgraph_static cypher-parser-build )

set_target_properties ( redisgraph_static PROPERTIES
	VERSION ${RedisGraph_VERSION_MAJOR}.${RedisGraph_VERSION_MINOR}.${RedisGraph_VERSION_SUB}
	LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build
	SOVERSION ${RedisGraph_VERSION_MAJOR}
	C_STANDARD_REQUIRED 11 )
set_property ( TARGET redisgraph_static PROPERTY C_STANDARD 11 )

#-------------------------------------------------------------------------------
# link libraries
#-------------------------------------------------------------------------------

# rax
target_link_libraries( redisgraph_static PRIVATE ${CMAKE_SOURCE_DIR}/deps/rax/rax.o )
# xxhash
target_link_libraries( redisgraph_static PRIVATE ${CMAKE_SOURCE_DIR}/deps/xxHash/libxxhash.a )
# GraphBLAS
target_link_libraries ( redisgraph_static PUBLIC ${CMAKE_SOURCE_DIR}/deps/GraphBLAS/build/libgraphblas.a )
# redisearch
target_link_libraries( redisgraph_static PRIVATE ${CMAKE_SOURCE_DIR}/bin/macos-arm64v8-release/search-static/libredisearch.a )
# libcypher-parser
target_link_libraries( redisgraph_static PRIVATE ${CMAKE_SOURCE_DIR}/deps/libcypher-parser/lib/src/.libs/libcypher-parser.a )

