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

option(BUILD_STATIC "Make a static build of libmusly" OFF)
if (BUILD_STATIC)
    set(BUILD_SHARED_LIBS OFF)

    # remove -Wl,-Bdynamic
    set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)
    set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
else ()
    set(BUILD_SHARED_LIBS ON)
endif ()

option(BUILD_TEST "Build selftest executable" OFF)

option(USE_OPENMP "Compile with OpenMP support (Parallelization)" OFF)
if (USE_OPENMP)
    find_package(OpenMP)
    if (OPENMP_FOUND)
        set(CMAKE_C_FLAGS
            "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set(CMAKE_CXX_FLAGS
            "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS
            "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")
        set(CMAKE_SHARED_LINKER_FLAGS
            "${CMAKE_SHARED_LINKER_FLAGS} ${OpenMP_CXX_FLAGS}")
        # do not set CMAKE_STATIC_LINKER_FLAGS; will not work
    endif()
endif ()

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

