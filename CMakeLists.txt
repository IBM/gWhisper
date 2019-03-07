# Copyright 2019 IBM Corporation
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required (VERSION 2.8)
project (gWhisper)

if(DEFINED CMAKE_BUILD_TYPE)
else()
    # By default we build in Debug mode. For releases,
    # please run CMAKE with
    # -DCMAKE_BUILD_TYPE=RelWithDebInfo

    # non optimized with debug symbols:
    set(CMAKE_BUILD_TYPE Debug)
endif()

if (CMAKE_VERSION VERSION_LESS "3.1")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
    else()
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    endif()
else()
    set(CMAKE_CXX_STANDARD 11)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
enable_testing()

if(BUILD_CONFIG_USE_BOOST_REGEX)
    add_definitions(-DBUILD_CONFIG_USE_BOOST_REGEX)
endif()

# this causes all built executables to be on build directory toplevel.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_BINARY_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/src")

add_subdirectory(src)

if(EXISTS "${PROJECT_SOURCE_DIR}/third_party/googletest/CMakeLists.txt")
    # we only build tests if googletest is available
    # If the gWhisper source package is downloaded from GitHub as archive (tar or zip)
    # submodules are not included. This ensures, that we build at least the functional
    # code in this case.
    add_subdirectory(tests)
else()
    message(WARNING "googletest submodule not found, not building tests. Please be sure to get the submodules with 'git submodule update --init' to also build tests.")
endif()

add_subdirectory(third_party)
