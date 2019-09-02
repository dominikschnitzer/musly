# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.
#
# (c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
#          2014, Jan Schlueter <jan.schlueter@ofai.at>

cmake_minimum_required(VERSION 3.5)

project(musly)
set(MUSLY_VERSION "0.2")
add_definitions(-DMUSLY_VERSION="${MUSLY_VERSION}")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (MSVC)
    # Ninja uses those flags on Windows, MSBuild ignores them
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.14)
        add_compile_options(/experimental:external /external:W0)
        set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "/external:I")
        set(CMAKE_INCLUDE_SYSTEM_FLAG_C "/external:I")
    endif()

    # warning level 4 and all warnings as errors
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif()
    if(CMAKE_C_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    endif()
    add_compile_options(/WX)

    add_compile_definitions(NOMINMAX _USE_MATH_DEFINES)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

    set(PREFER_STATIC ON)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Werror
        $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
        $<$<COMPILE_LANGUAGE:CXX>:-pedantic>)
    # On Unix, hide symbols by default (see https://gcc.gnu.org/wiki/Visibility)
    add_compile_options(-fvisibility=hidden
        $<$<COMPILE_LANGUAGE:CXX>:-fvisibility-inlines-hidden>)

    set(PREFER_STATIC OFF)
endif()

option(BUILD_STATIC "Make a static build of libmusly" PREFER_STATIC)
if (BUILD_STATIC)
    set(BUILD_SHARED_LIBS OFF)

    # remove -Wl,-Bdynamic
    set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)
    set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
else ()
    set(BUILD_SHARED_LIBS ON)
endif ()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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

add_subdirectory(external)
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

