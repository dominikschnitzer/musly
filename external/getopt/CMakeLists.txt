# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.

# (c) 2019, Dominik Schnitzer <dominik@schnitzer.at>

add_library(getopt_windows STATIC
    getopt.c)

target_include_directories(getopt_windows INTERFACE
    ${CMAKE_CURRENT_LIST_DIR})

target_compile_options(getopt_windows PRIVATE
    /W4)
