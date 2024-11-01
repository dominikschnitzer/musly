# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.

# (c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>


add_library(libkissfft STATIC
    src/kiss_fft.c
    src/kiss_fftr.c
)

target_include_directories(libkissfft 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
        $<INSTALL_INTERFACE:include>
)

set_target_properties(libkissfft PROPERTIES
    # do not add a 'lib' prefix to the target name a.k.a. liblibresample.a
    PREFIX ""
    # always ouput position independent code
    POSITION_INDEPENDENT_CODE ON
    # disable comiler specific extensions
    C_EXTENSIONS OFF
)

# enable IPO when compiler supports it
if(musly_IPO_SUPPORTED)
    set_target_properties(libkissfft PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION TRUE
    )
endif()   
