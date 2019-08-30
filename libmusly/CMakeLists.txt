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

# configure libresample
find_file(HAVE_INTTYPES inttypes.h)
configure_file(libresample/config.h.in libresample/config.h)

if(EXISTS "${LIBAV_INCLUDE_DIRS}/libavutil/channel_layout.h")
    set_source_files_properties(decoders/libav.cpp
        PROPERTIES COMPILE_FLAGS "-DHAVE_AVUTIL_CHANNEL_LAYOUT")
endif()

if(USE_OPENMP AND OPENMP_FOUND)
    # disable OpenMP for kiss FFT, it slows things down terribly
    set_source_files_properties(kissfft/kiss_fft.c
        PROPERTIES COMPILE_FLAGS "-U_OPENMP")
endif()

include_directories(
    ${LIBMUSLY_INCLUDE}
    ${CMAKE_CURRENT_SOURCE_DIR}
)


# KissFFT
set(KISSFFT_C
    kissfft/kiss_fft.c
    kissfft/kiss_fftr.c
)
set(KISSFFT_H
    kissfft/kiss_fft.h
    kissfft/kiss_fftr.h
)
add_library(kissfft STATIC
    ${KISSFFT_C}
    ${KISSFFT_H}
)
if (MSVC)
    # kissfft does not compile with warning level>2
    target_compile_options(kissfft PRIVATE
        /W2)
else()
    target_compile_options(kissfft PRIVATE
        -pedantic -Wextra)
endif()


# libresample
set(LIBRESAMPLE_C
    libresample/filterkit.c
    libresample/resamplesubs.c
    libresample/resample.c
)
set(LIBRESAMPLE_H
    libresample/filterkit.h
    libresample/libresample.h
    libresample/resample_defs.h
    libresample/config.h.in
)
add_library(libresample STATIC
    ${LIBRESAMPLE_C}
    ${LIBRESAMPLE_H}
)
if (MSVC)
    # libresample does not compile with warning level>1
    target_compile_options(libresample PRIVATE
        /W1)
else()
    target_compile_options(kissfft PRIVATE
        -pedantic)
endif()


# libmusly
set(LIBMUSLY_CPP
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
)
set(LIBMUSLY_METHODS_CPP
    methods/mandelellis.cpp
    methods/timbre.cpp
)
set(LIBMUSLY_METHODS_H
    methods/mandelellis.h
    methods/timbre.h
)
set(LIBMUSLY_DECODERS_CPP
    decoders/libav.cpp
)
set(LIBMUSLY_DECODERS_H
    decoders/libav.h
)
set(LIBMUSLY_H
    resampler.h
    plugins.h
    method.h
    decoder.h
    windowfunction.h
    powerspectrum.h
    melspectrum.h
    discretecosinetransform.h
    mfcc.h
    gaussianstatistics.h
    mutualproximity.h
)

add_library(libmusly
    ${LIBMUSLY_CPP}
    ${LIBMUSLY_H}
    ${LIBMUSLY_METHODS_CPP}
    ${LIBMUSLY_METHODS_H}
    ${LIBMUSLY_DECODERS_CPP}
    ${LIBMUSLY_DECODERS_H}
)

target_include_directories(libmusly
    SYSTEM
    PRIVATE
    ${EIGEN3_INCLUDE_DIR}
    ${LIBAV_INCLUDE_DIRS}
)

source_group("libmusly" FILES
    ${LIBMUSLY_CPP}
    ${LIBMUSLY_H}
)
source_group("libmusly\\methods" FILES
    ${LIBMUSLY_METHODS_CPP}
    ${LIBMUSLY_METHODS_H}
)
source_group("libmusly\\decoders" FILES
    ${LIBMUSLY_DECODERS_CPP}
    ${LIBMUSLY_DECODERS_H}
)
source_group("kissfft" FILES
    ${KISSFFT_C}
    ${KISSFFT_H}
)
source_group("libresample" FILES
    ${LIBRESAMPLE_C}
    ${LIBRESAMPLE_H}
)

if(BUILD_STATIC)
    target_compile_definitions(libmusly PRIVATE
        BUILD_STATIC MUSLY_BUILDING_STATIC)
else()
    # This defines MUSLY_BUILDING_LIBRARY *only when building as shared library*,
    # allowing us to use __declspec(dllexport) instead of dllimport on Windows.
    target_compile_definitions(libmusly PRIVATE
        MUSLY_BUILDING_LIBRARY)
endif()

target_link_libraries(libmusly
    kissfft
    libresample
    ${LIBMUSLY_LIBS}
    ${LIBAV_LIBRARIES})
if(WIN32 OR MINGW)
    # link against winsock2 for ntohl() and htonl()
    target_link_libraries(libmusly
        ws2_32)
endif()

set_target_properties(libmusly
    PROPERTIES PREFIX "")

install(TARGETS libmusly
    DESTINATION lib)
