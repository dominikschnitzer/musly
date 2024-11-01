# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.
#
# (c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
#          2014, Jan Schlueter <jan.schlueter@ofai.at>

cmake_minimum_required(VERSION 3.24)

project(musly
    VERSION 0.2
    LANGUAGES C CXX
)

# disable in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt. ")
endif()

# set default build type
set(default_build_type "Release")
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
    
    set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
        STRING "Choose the type of build." FORCE
    )
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY 
        STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
    )
endif()

# determine if we get imported by another cmake project
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(IS_SUBPROJECT OFF)
else()
    set(IS_SUBPROJECT ON)
endif()

option(BUILD_STATIC "Make a static build of libmusly" OFF)
if (BUILD_STATIC)
    set(BUILD_SHARED_LIBS OFF)
    message(STATUS "Building static libmusly")
else ()
    set(BUILD_SHARED_LIBS ON)
    message(STATUS "Building shared libmusly")
endif ()

option(BUILD_TEST "Build selftest executable" OFF)


#
# Project Setup
#

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CheckIPOSupported)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

if(NOT IS_SUBMODULE)
    # setup c++
    set(CMAKE_C_EXTENSIONS OFF)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    set(CMAKE_POSITION_INDEPENDENT_CODE ON)

    set(CMAKE_INCLUDE_CURRENT_DIR OFF)

    # add support for folders IDE project generators
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

set(musly_CMAKEPKG_INSTALL_DIR 
    "${CMAKE_INSTALL_DATADIR}/musly/cmake"
    CACHE PATH "The directory relative to CMAKE_INSTALL_PREFIX where the Musly CMake package files are installed.")

check_ipo_supported(RESULT musly_IPO_SUPPORTED)


#
# Dependencies
#

# make sure Eigen3 library is present
include(FetchContent)
FetchContent_Declare(Eigen3
    URL      https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz
    URL_HASH SHA256=8586084f71f9bde545ee7fa6d00288b264a2b7ac3607b974e54d13e7162c1c72
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    EXCLUDE_FROM_ALL
    FIND_PACKAGE_ARGS NO_MODULE
)

FetchContent_MakeAvailable(Eigen3)

# can be replaced with FetchContent_MakeAvailable(Eigen3), when CMake >= 3.28
#FetchContent_GetProperties(Eigen3)
#if(NOT eigen3_POPULATED)
#  FetchContent_Populate(Eigen3)
#  add_subdirectory(${eigen3_SOURCE_DIR} ${eigen3_BINARY_DIR} EXCLUDE_FROM_ALL)
#endif()

add_subdirectory(libkissfft)
add_subdirectory(libresample)
add_subdirectory(libmusly)

if (NOT IS_SUBPROJECT)
    if (NOT WIN32 AND NOT MINGW)
        add_subdirectory(musly)
    endif()

    if (BUILD_TEST)
        enable_testing()
        add_subdirectory(test)
    endif ()

    # Documentation
    set(musly_DOC_FILES AUTHORS COPYING README.md)
    set(musly_DOC_PATH "share/doc/musly")
    install(FILES ${musly_DOC_FILES}
        DESTINATION ${musly_DOC_PATH})

endif()

#
# Export Configuration
#

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MuslyConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/MuslyConfig.cmake
    INSTALL_DESTINATION ${musly_CMAKEPKG_INSTALL_DIR}
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/MuslyConfigVersion.cmake"
    VERSION "${musly_VERSION_MAJOR}.${musly_VERSION_MINOR}"
    COMPATIBILITY SameMajorVersion
)

export(EXPORT MuslyTargets
    FILE MuslyTargets.cmake
    NAMESPACE Musly::
)

install(EXPORT MuslyTargets
    FILE MuslyTargets.cmake
    NAMESPACE Musly::
    DESTINATION ${musly_CMAKEPKG_INSTALL_DIR}
)

export(PACKAGE Musly)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/MuslyConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/MuslyConfigVersion.cmake
    DESTINATION ${musly_CMAKEPKG_INSTALL_DIR}
)

#
# Packaging
#

if(NOT IS_SUBMODULE)
    set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

    set(CPACK_SOURCE_GENERATOR "TGZ;ZIP")
    set(CPACK_SOURCE_IGNORE_FILES
        /.git
        /.vscode
        /.travis\\\\.yml
        /\\\\.github
        /dist
        /.*build.*
        /\\\\.DS_Store
    )
    include(CPack)
endif()


#
# Uninstall
#

# Add uninstall target
add_custom_target(uninstall_musly
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MuslyUninstall.cmake
)
