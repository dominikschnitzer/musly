# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.
#
# (c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
#          2014, Jan Schlueter <jan.schlueter@ofai.at>

cmake_minimum_required(VERSION 3.17)

project(musly)
include(GNUInstallDirs)
include(CTest)
set(MUSLY_VERSION "0.2")
add_definitions(-DMUSLY_VERSION="${MUSLY_VERSION}")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-Wall -g)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_definitions(-DNDEBUG -O3)
else ()
    add_definitions(-DNDEBUG -Wall -g -O3)
endif ()

# On Unix, hide symbols by default (see https://gcc.gnu.org/wiki/Visibility)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"
    OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
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

find_package(PkgConfig REQUIRED)
pkg_check_modules(EIGEN3 REQUIRED IMPORTED_TARGET eigen3)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
    libavcodec
    libavformat
    libavutil)
pkg_check_modules(LIBRESAMPLE REQUIRED IMPORTED_TARGET libresample)
pkg_search_module(KISSFFT REQUIRED IMPORTED_TARGET
    kissfft-simd
    kissfft-double
    kissfft-float
    kissfft-int32_t
    kissfft-int16_t
    kissfft-simd-openmp
    kissfft-double-openmp
    kissfft-float-openmp
    kissfft-int32_t-openmp
    kissfft-int16_t-openmp)

include_directories(
    "${PROJECT_SOURCE_DIR}/include")

add_subdirectory(libmusly) 
add_subdirectory(musly) 
add_subdirectory(include) 
if (BUILD_TESTING)
    add_subdirectory(test)
endif ()

# Documentation
set(musly_DOC_FILES AUTHORS COPYING README.md)
install(FILES ${musly_DOC_FILES} DESTINATION ${CMAKE_INSTALL_DOCDIR})
