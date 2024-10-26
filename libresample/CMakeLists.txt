# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.

# (c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>

find_file(HAVE_INTTYPES inttypes.h)
configure_file(src/config.h.in config.h)

add_library(libresample STATIC
    src/filterkit.c
    src/resamplesubs.c
    src/resample.c
)

target_include_directories(libresample 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}
    INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
        $<INSTALL_INTERFACE:include>
)

set_target_properties(libresample PROPERTIES
    # do not add a 'lib' prefix to the target name a.k.a. liblibresample.a
    PREFIX ""
    # always ouput position independent code
    POSITION_INDEPENDENT_CODE ON
    # disable comiler specific extensions
    C_EXTENSIONS OFF
)

# enable IPO when compiler supports it
if(musly_IPO_SUPPORTED)
    set_target_properties(libresample PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION TRUE
    )
endif()   
