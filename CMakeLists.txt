# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.
#
# (c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
#          2014, Jan Schlueter <jan.schlueter@ofai.at>

cmake_minimum_required(VERSION 2.8)

project(musly)
set(MUSLY_VERSION "0.2")
add_definitions(-DMUSLY_VERSION="${MUSLY_VERSION}")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-Wall -g)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DNDEBUG -O3)
else ()
    add_definitions(-DNDEBUG -Wall -g -O3)
endif () 

option(BUILD_STATIC "Make a static build" OFF)
if (BUILD_STATIC)
    set(BUILD_SHARED_LIBS OFF)

    # remove -Wl,-Bdynamic
    set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)
    set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
else ()
    set(BUILD_SHARED_LIBS ON)
endif ()

option(BUILD_TEST "Build selftest executable" OFF)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(Eigen3 REQUIRED)
find_package(LibAV 0.8 COMPONENTS avcodec avformat avutil REQUIRED)

include_directories(
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/include")

add_subdirectory(libmusly) 
add_subdirectory(musly) 
add_subdirectory(include) 
if (BUILD_TEST)
    enable_testing()
    add_subdirectory(test)
endif ()

# Documentation
set(musly_DOC_FILES AUTHORS COPYING README.md)
set(musly_DOC_PATH "share/doc/musly")
install(FILES ${musly_DOC_FILES}
        DESTINATION ${musly_DOC_PATH})

