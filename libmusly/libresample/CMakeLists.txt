# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.

# (c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>

find_file(HAVE_INTTYPES inttypes.h)
configure_file(config.h.in config.h)

add_library(musly_resample
    filterkit.c
    resamplesubs.c
    resample.c)

target_include_directories(musly_resample PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

set(LIBMUSLY_LIBS
    ${LIBMUSLY_LIBS}
    musly_resample
    PARENT_SCOPE)

install(TARGETS musly_resample
    DESTINATION lib)
