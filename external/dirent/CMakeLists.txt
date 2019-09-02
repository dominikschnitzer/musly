# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.

# (c) 2019, Dominik Schnitzer <dominik@schnitzer.at>

add_library(dirent_windows INTERFACE)
target_include_directories(dirent_windows INTERFACE
    ${CMAKE_CURRENT_LIST_DIR})
