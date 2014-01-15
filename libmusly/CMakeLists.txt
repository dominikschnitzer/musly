# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.

# (c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>

add_subdirectory(
    libresample)

if(EXISTS "external" AND IS_DIRECTORY "external")
    add_subdirectory(
        external)
endif()

include_directories(
    ${EIGEN3_INCLUDE_DIR}
    ${LIBAV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR})

add_library(libmusly
    kissfft/kiss_fft.c
    kissfft/kiss_fftr.c
    methods/mandelellis.cpp
    methods/timbre.cpp
    decoders/libav_0_8.cpp
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
    lib.cpp)

target_link_libraries(libmusly
    ${LIBMUSLY_LIBS}
    ${LIBAV_LIBRARIES})

set_target_properties(libmusly
    PROPERTIES PREFIX "")

install(TARGETS libmusly
    DESTINATION lib)
