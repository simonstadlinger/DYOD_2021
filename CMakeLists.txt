cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(OpossumDB)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.2)
        message(FATAL_ERROR "Your GCC version ${CMAKE_CXX_COMPILER_VERSION} is too old.")
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.0)
        message(FATAL_ERROR "Your clang version ${CMAKE_CXX_COMPILER_VERSION} is too old.")
    endif()
else()
    message(WARNING "You are using an unsupported compiler (${CMAKE_CXX_COMPILER_ID})! Compilation has only been tested with Clang and GCC.")
endif()

# Enable address and undefined behavior sanitization if requested
option(ENABLE_ADDR_UB_SANITIZATION "Set to ON to build Hyrise with ASAN and UBSAN enabled. Default: OFF" OFF)
if (${ENABLE_ADDR_UB_SANITIZATION})
    # add_compile_options() wants list, CMAKE_EXE_LINKER_FLAGS a string. There are probably cleverer ways than
    # duplicating the flags, but this is probably the simplest solution.
    # -fno-sanitize-recover=all is used to make UBsan fail (i.e., return a non-zero exit code) when an error is found.
    add_compile_options(-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer")
endif()

# Enable thread sanitization if requested
option(ENABLE_THREAD_SANITIZATION "Set to ON to build Hyrise with TSAN enabled. Default: OFF" OFF)
if (${ENABLE_THREAD_SANITIZATION})
    # add_compile_options() wants list, CMAKE_EXE_LINKER_FLAGS a string. There are probably cleverer ways than
    # duplicating the flags, but this is probably the simplest solution.
    add_compile_options(-fsanitize=thread -O1)
    add_link_options(-fsanitize=thread)
    set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread")
endif()

# Set default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

find_package(Boost REQUIRED)

# CMake settings
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH}) # To allow CMake to locate our Find*.cmake files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}) # Put binaries into root of build tree
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # Put libraries into their own dir
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native")
set(CMAKE_CXX_STANDARD 20)

# Include sub-CMakeLists.txt
add_subdirectory(third_party/ EXCLUDE_FROM_ALL)
add_subdirectory(third_party/googletest EXCLUDE_FROM_ALL)
add_subdirectory(src)


# Useful for printing all c++ files in a directory:
# find . -type f -name "*.cpp" -o -name "*.hpp" | cut -c 3- | sort
