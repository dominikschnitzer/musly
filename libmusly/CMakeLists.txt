add_library(libmusly)
add_library(Musly::libmusly ALIAS libmusly)

# use c++17 standard features
target_compile_features(libmusly
    PUBLIC cxx_std_17
)

# force msvc to conform with c++ standards
target_compile_options(libmusly
    PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->"
)

# fix preprocessor weirdness on windows
if(WIN32 OR MINGW)
    target_compile_definitions(libmusly
        PRIVATE 
            -DNOMINMAX
            -D_USE_MATH_DEFINES
    )
endif()

set_target_properties(libmusly PROPERTIES
    # do not add a 'lib' prefix to the target name a.k.a. liblibmusly.so
    PREFIX ""
    # always ouput position independent code
    POSITION_INDEPENDENT_CODE ON
    # disable comiler specific extensions
    CXX_EXTENSIONS OFF
    # name of preprocessor symbol that indicates that we are currently building a shared library
    # handy to determine if (dllexport or dllimport) should be used in public headers
    DEFINE_SYMBOL MUSLY_BUILDING_LIBRARY
)

# enable IPO when compiler supports it
if(musly_IPO_SUPPORTED)
    set_target_properties(libmusly PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION TRUE
    )
endif()   

# only use MPL2 code from Eigen library
target_compile_definitions(libmusly
    PRIVATE 
        -DEIGEN_MPL2_ONLY
)

# provide preprocessor symbol when we build a static library
if(BUILD_STATIC)
    target_compile_definitions(libmusly
        PUBLIC -DMUSLY_STATIC
    )
endif()

# create header file containing the current project version
configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/src/version.h"
)

# set include directories
target_include_directories(libmusly
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}/src
    INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# add common source files
target_sources(libmusly
    PRIVATE
        src/decoders/none.cpp
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

# add common link libraries
target_link_libraries(libmusly
    PRIVATE
        libkissfft
        libresample
        Eigen3::Eigen
)

# enable libav decoder support when available
find_package(LibAV 0.8 COMPONENTS avcodec avformat avutil)
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
elseif(LINUX)
    message(WARNING "No decoder library found! You'll only be able to analyze already decoded audio data")
endif()

# enable coreaudio decoder support when building for macos
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

# enable mediafoundation decoder support when building for windows
if(WIN32 OR MINGW)
    target_sources(libmusly 
        PRIVATE src/decoders/mediafoundation.cpp
    )
    target_link_libraries(libmusly
        PRIVATE
            Mf
            Mfplat
            mfreadwrite
            mfuuid
            ole32
            ws2_32
    )
    target_compile_definitions(libmusly
        PRIVATE -DMUSLY_DECODER_MEDIAFOUNDATION
    )
endif()


install(TARGETS libmusly 
    EXPORT MuslyTargets
    LIBRARY DESTINATION  ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION  ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION  ${CMAKE_INSTALL_BINDIR}
    FILE_SET HEADERS DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
