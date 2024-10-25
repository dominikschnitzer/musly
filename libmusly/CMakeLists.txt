# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.

# (c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>
#     2014-2016, Jan Schlueter <jan.schlueter@ofai.at>
#

add_library(libmusly)
add_library(Musly::libmusly ALIAS libmusly)

configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/src/version.h"
)

target_sources(libmusly
    PRIVATE
        src/kissfft/kiss_fft.c
        src/kissfft/kiss_fftr.c
        src/methods/mandelellis.cpp
        src/methods/timbre.cpp
        src/resampler.cpp
        src/plugins.cpp
        src/method.cpp
        src/decoder.cpp
        src/windowfunction.cpp
        src/powerspectrum.cpp
        src/melspectrum.cpp
        src/discretecosinetransform.cpp
        src/mfcc.cpp
        src/gaussianstatistics.cpp
        src/mutualproximity.cpp
        src/lib.cpp
    PUBLIC 
        FILE_SET HEADERS
            BASE_DIRS 
                include 
            FILES
                include/musly/musly.h
                include/musly/musly_types.h
)

target_include_directories(libmusly
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}/src
    INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
)

target_link_libraries(libmusly
    PRIVATE
        resample 
        Eigen3::Eigen
)

target_compile_features(libmusly
    PUBLIC cxx_std_11
)

set_target_properties(libmusly PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    CXX_EXTENSIONS OFF
    DEFINE_SYMBOL MUSLY_BUILDING_LIBRARY
)

if(musly_IPO_SUPPORTED)
    set_target_properties(resample PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION TRUE
    )
endif()   

target_compile_definitions(libmusly
    PRIVATE 
        -DEIGEN_MPL2_ONLY
)

if(BUILD_STATIC)
    target_compile_definitions(libmusly
        PUBLIC -DMUSLY_STATIC
    )
endif()

if (LINUX)
    find_package(LibAV 0.8 COMPONENTS avcodec avformat avutil REQUIRED)
else()
    find_package(LibAV 0.8 COMPONENTS avcodec avformat avutil)
endif()

if(LIBAV_FOUND)
    target_sources(libmusly
        PRIVATE src/decoders/libav.cpp
    )
    target_link_libraries(libmusly
        PRIVATE ${LIBAV_LIBRARIES}
    )
    target_include_directories(libmusly
        PRIVATE ${LIBAV_INCLUDE_DIRS}
    )
    target_compile_definitions(libmusly 
        PRIVATE -DMUSLY_DECODER_LIBAV
    )

    if(EXISTS "${LIBAV_INCLUDE_DIRS}/libavutil/channel_layout.h")
        set_source_files_properties(decoders/libav.cpp PROPERTIES
            COMPILE_FLAGS "-DHAVE_AVUTIL_CHANNEL_LAYOUT"
        )
    endif()
endif()

if(APPLE)
    find_library(AUDIOTOOLBOX_LIBRARY AudioToolbox)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    mark_as_advanced(AUDIOTOOLBOX_LIBRARY COREFOUNDATION_LIBRARY)

    target_sources(libmusly 
        PRIVATE src/decoders/coreaudio.cpp
    )
    target_link_libraries(libmusly
        PRIVATE
            ${AUDIOTOOLBOX_LIBRARY}
            ${COREFOUNDATION_LIBRARY}
    )
    target_compile_definitions(libmusly
        PRIVATE -DMUSLY_DECODER_COREAUDIO
    )
endif()

if(WIN32 OR MINGW)
    target_sources(libmusly 
        PRIVATE src/decoders/mediafoundation.cpp
    )
    target_link_libraries(libmusly
        PRIVATE
            ws2_32
            Mf
            Mfplat
            mfreadwrite
            mfuuid
            ole32
    )
    target_compile_definitions(libmusly
        PRIVATE 
            -DMUSLY_DECODER_MEDIAFOUNDATION 
            -DNOMINMAX
            -D_USE_MATH_DEFINES
    )
endif()

if(APPLE)
  set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
elseif(UNIX)
  set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif()

install(TARGETS libmusly 
    EXPORT MuslyTargets
    LIBRARY DESTINATION  ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION  ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION  ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
