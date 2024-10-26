# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.

# (c) 2013, Dominik Schnitzer <dominik@schnitzer.at>

include_directories(
    Eigen3::Eigen
)

add_executable(musly
    tools.cpp
    fileiterator.cpp
    programoptions.cpp
    collectionfile.cpp
    main.cpp
)

target_link_libraries(musly
    libmusly
    Eigen3::Eigen
)

install(TARGETS musly DESTINATION ${CMAKE_INSTALL_BINDIR})
