cmake_minimum_required(VERSION 3.16)
project(HILL_CACHE)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Debug build")
    # 添加一些只在 Debug 模式下执行的操作
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Release build")
    # 添加一些只在 Release 模式下执行的操作
else()
    message("Unknown build type")
endif()

#IF(CMAKE_BUILD_TYPE STREQUAL "Debug" OR DEBUG)
#    MESSAGE("DEBUG has been set as TRUE ${DEBUG}")
#    SET(CMAKE_COMMON_FLAGS "${CMAKE_COMMON_FLAGS} -O0 -g -DDEBUG")
#    ADD_DEFINITIONS(-DENABLE_DEBUG)
#ELSEIF(NOT DEFINED ENV{DEBUG})
#    MESSAGE("Disable debug")
#    SET(CMAKE_COMMON_FLAGS "${CMAKE_COMMON_FLAGS} -O2")
#    SET(CMAKE_BUILD_TYPE "Release")
#ELSE()
#    MESSAGE("Enable debug")
#    SET(CMAKE_COMMON_FLAGS "${CMAKE_COMMON_FLAGS} -O0 -g -DDEBUG")
#    ADD_DEFINITIONS(-DENABLE_DEBUG)
#ENDIF()

find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)

if(CLANG_FORMAT_EXECUTABLE)
    message(STATUS "ClangFormat found: ${CLANG_FORMAT_EXECUTABLE}")
    file(GLOB_RECURSE source_files
        ${CMAKE_SOURCE_DIR}/src/*.cpp
        ${CMAKE_SOURCE_DIR}/include/*.h
        )
    add_custom_target(
        format
        COMMAND ${CLANG_FORMAT_EXECUTABLE}
        -i
        ${source_files}
        COMMENT "Running ClangFormat"
    )
#    message("${source_files}")
    message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
    message(STATUS "CLANG_FORMAT_EXECUTABLE: ${CLANG_FORMAT_EXECUTABLE}")
    
    add_dependencies(format format)
endif()


MESSAGE("CMAKE_COMMON_FLAGS is " ${CMAKE_COMMON_FLAGS})
SET(CMAKE_CXX_FLAGS ${CMAKE_COMMON_FLAGS})
SET(CMAKE_C_FLAGS ${CMAKE_COMMON_FLAGS})
MESSAGE("CMAKE_CXX_FLAGS is " ${CMAKE_CXX_FLAGS})

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/replacers)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/util)
#INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/benchmark)
#INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include/prototype)

ADD_SUBDIRECTORY(src)

enable_testing()

# find_program(LCOV lcov REQUIRED)
# find_program(GENHTML genhtml REQUIRED)

# ADD_EXECUTABLE(rocksdb_use rocksdb_use.cpp)
# target_link_libraries(rocksdb_use rocksdb memcached)

#ADD_EXECUTABLE(rocksdb_use_multi_threads src/benchmark/rocksdb_use_multi_threads.cpp)
#target_link_libraries(rocksdb_use_multi_threads pthread rocksdb memcached)

add_custom_target(coverage
        COMMAND ${LCOV} --directory . --capture --output-file coverage.info
        COMMAND ${LCOV} --extract coverage.info '*/src/*' --output-file coverage2.info
        COMMAND ${GENHTML} --demangle-cpp -o coverage coverage2.info)