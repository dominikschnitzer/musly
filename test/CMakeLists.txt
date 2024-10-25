# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.

# (c) 2014, Jan Schlueter <jan.schlueter@ofai.at>

include_directories(
    "${PROJECT_SOURCE_DIR}/libmusly/src"
    "${PROJECT_SOURCE_DIR}/musly"
)

add_executable(selftest
    "${PROJECT_SOURCE_DIR}/musly/tools.cpp"
    main.cpp
)

target_link_libraries(selftest
    libmusly
)

add_test(NAME selftest 
    COMMAND selftest
)
