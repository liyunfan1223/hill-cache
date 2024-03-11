cmake_minimum_required(VERSION 3.16)
PROJECT(main)

SET(CMAKE_COMMON_FLAGS  -coverage)
MESSAGE("MAIN CMAKE_COMMON_FLAGS ${CMAKE_COMMON_FLAGS}")

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(./data_structures)

FILE(GLOB_RECURSE ALL_SRC replacers/*.cpp util/*.cpp)
FILE(GLOB MAIN_SRC proto.cpp)
#MESSAGE("MAIN SRC: " ${MAIN_SRC})
FOREACH (F ${ALL_SRC})
    SET(LIB_SRC ${LIB_SRC} ${F})
ENDFOREACH (F)

# MESSAGE("LIB_SRC: ${LIB_SRC}")
ADD_EXECUTABLE(proto proto.cpp)
TARGET_LINK_LIBRARIES(proto main_static)
set_target_properties(proto PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

find_library(pthread_FOUND pthread)
find_library(rocksdb_FOUND rocksdb)
find_library(memcached_FOUND memcached)

set(LIBS_FOUND TRUE)
if(NOT pthread_FOUND)
    message(WARNING "pthread library not found")
    set(LIBS_FOUND FALSE)
endif()

if(NOT rocksdb_FOUND)
    message(WARNING "rocksdb library not found")
    set(LIBS_FOUND FALSE)
endif()

if(NOT rocksdb_FOUND)
    message(WARNING "rocksdb library not found")
    set(LIBS_FOUND FALSE)
endif()

if (LIBS_FOUND)
    ADD_EXECUTABLE(bench bench.cpp)
    TARGET_LINK_LIBRARIES(bench pthread rocksdb memcached)
    set_target_properties(bench PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
endif()

MESSAGE("CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

ADD_LIBRARY(main_static STATIC ${LIB_SRC})