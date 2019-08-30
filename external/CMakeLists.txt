# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.

# (c) 2019, Dominik Schnitzer <dominik@schnitzer.at>

if(WIN32)
    add_subdirectory(dirent)
    add_subdirectory(getopt)
endif()
