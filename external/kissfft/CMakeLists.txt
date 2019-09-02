# CMake buildfile generator file.
# Process with cmake to create your desired buildfiles.

# (c) 2019, Dominik Schnitzer <dominik@schnitzer.at>

# KissFFT

set(KISSFFT_C
    kiss_fft.c
    kiss_fftr.c
)
set(KISSFFT_H
    kiss_fft.h
    kiss_fftr.h
)
add_library(kissfft STATIC
    ${KISSFFT_C}
    ${KISSFFT_H}
)
source_group("kissfft" FILES
    ${KISSFFT_C}
    ${KISSFFT_H}
)

if (MSVC)
    # kissfft does not compile with warning level>2
    target_compile_options(kissfft PRIVATE
        /W2)
else()
    target_compile_options(kissfft PRIVATE
        -pedantic -Wextra)
endif()
if(USE_OPENMP AND OPENMP_FOUND)
    # disable OpenMP for kiss FFT, it slows things down terribly
    target_compile_options(kissfft PRIVATE -U_OPENMP)
endif()
target_include_directories(kissfft PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR})
