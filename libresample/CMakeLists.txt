# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.

# (c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>

find_file(HAVE_INTTYPES inttypes.h)
configure_file(src/config.h.in config.h)

add_library(resample STATIC
    src/filterkit.c
    src/resamplesubs.c
    src/resample.c
)

target_include_directories(resample 
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}
    INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
        $<INSTALL_INTERFACE:include>
)

set_target_properties(resample PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    C_EXTENSIONS OFF
)

if(musly_IPO_SUPPORTED)
    set_target_properties(resample PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION TRUE
    )
endif()   