# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.

# (c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>

add_library(musly_resample
    filterkit.c
    resamplesubs.c
    resample.c)

set(LIBMUSLY_LIBS
    ${LIBMUSLY_LIBS}
    musly_resample
    PARENT_SCOPE)

install(TARGETS musly_resample
    DESTINATION lib)
