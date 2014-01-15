# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.

# (c) 2013-2014, Dominik Schnitzer <dominik@schnitzer.at>

add_library(libmusly_resample
    filterkit.c
    resamplesubs.c
    resample.c)

set_target_properties(libmusly_resample
    PROPERTIES PREFIX "")

set(LIBMUSLY_LIBS
    ${LIBMUSLY_LIBS}
    libmusly_resample
    PARENT_SCOPE)

install(TARGETS libmusly_resample
    DESTINATION lib)
