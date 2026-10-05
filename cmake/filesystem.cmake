set(FILESYSTEM_LIB "")

# Check for the the need to link FS library
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.1)
        message(STATUS "Using GCC version less than 9.1, linking with -lstdc++fs")
        set(FILESYSTEM_LIB stdc++fs)
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
        message(STATUS "Using Clang version less than 9.0, linking with -lc++fs")
        set(FILESYSTEM_LIB c++fs)
    endif()
endif()