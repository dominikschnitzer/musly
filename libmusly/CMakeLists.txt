# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.

# (c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
#     2014-2016, Jan Schlueter <jan.schlueter@ofai.at>

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external")
    add_subdirectory(
        external)
    set_source_files_properties(${LIBMUSLY_EXTERNAL}
        PROPERTIES COMPILE_FLAGS "${LIBMUSLY_EXTERNAL_FLAGS}")
    set_source_files_properties(lib.cpp
        PROPERTIES COMPILE_FLAGS "-DLIBMUSLY_EXTERNAL ${LIBMUSLY_EXTERNAL_FLAGS}")
endif()

include_directories(
    ${LIBMUSLY_INCLUDE}
    ${CMAKE_CURRENT_SOURCE_DIR})

add_library(libmusly
    methods/mandelellis.cpp
    methods/timbre.cpp
    decoders/libav.cpp
    resampler.cpp
    plugins.cpp
    method.cpp
    decoder.cpp
    windowfunction.cpp
    powerspectrum.cpp
    melspectrum.cpp
    discretecosinetransform.cpp
    mfcc.cpp
    gaussianstatistics.cpp
    mutualproximity.cpp
    lib.cpp
    ${LIBMUSLY_EXTERNAL})

if(BUILD_STATIC)
    set_target_properties(libmusly
        PROPERTIES COMPILE_FLAGS "-DBUILD_STATIC")
endif()

# This defines MUSLY_BUILDING_LIBRARY *only when building as shared library*,
# allowing us to use __declspec(dllexport) instead of dllimport on Windows.
set_target_properties(libmusly
    PROPERTIES DEFINE_SYMBOL MUSLY_BUILDING_LIBRARY)

target_link_libraries(libmusly
    ${LIBMUSLY_LIBS}
    PkgConfig::EIGEN3
    PkgConfig::FFMPEG
    PkgConfig::LIBRESAMPLE
    PkgConfig::KISSFFT)
if(WIN32 OR MINGW)
    # link against winsock2 for ntohl() and htonl()
    target_link_libraries(libmusly ws2_32)
endif()

set_target_properties(libmusly
    PROPERTIES PREFIX "")

install(TARGETS libmusly)
