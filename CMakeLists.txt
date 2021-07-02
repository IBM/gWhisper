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

cmake_minimum_required (VERSION 3.14)
project (gWhisper)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
################################################################################
# BUILD TYPE AND COMPILER FLAGS

# Need to check if CMAKE_BUILD_TYPE is empty, it is always defined, but empty by default
if(NOT CMAKE_BUILD_TYPE)
    # By default we build in Debug mode. For releases,
    # please run CMAKE with
    # -DCMAKE_BUILD_TYPE=RelWithDebInfo

    # non optimized with debug symbols:
    message("Cmake Build type is not set setting: " ${CMAKE_BUILD_TYPE})
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()
message("Build Type is set to: " ${CMAKE_BUILD_TYPE})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
################################################################################

option(GWHISPER_BUILD_TESTSERVER "Build a test server, which may be used to play around with gWhisper. It is also used by automated functional tests." ON)

################################################################################
# BUILD TESTS
option(GWHISPER_BUILD_TESTS "Build unit and functional tests - needs active internet connection for googletest" OFF)
if(GWHISPER_BUILD_TESTS)
    enable_testing()
    message("Testing Enabled: Building unit and functional tests")
    message("Testing Enabled: Need access to github to be successful")
else()
    message("Testing Disabled")
endif()

################################################################################

option(GWHISPER_FORCE_BUILDING_GRPC "Fetch, build and statically link against gRPC even if gRPC is found to be installed" OFF)
include(gRPC)

if(BUILD_CONFIG_USE_BOOST_REGEX)
    add_definitions(-DBUILD_CONFIG_USE_BOOST_REGEX)
endif()

# this causes all built executables to be on build directory toplevel.
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

# avoid to compile again when make install
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)

add_subdirectory(third_party)
add_subdirectory(src)
add_subdirectory(tests)

install(FILES complete.bash DESTINATION /usr/share/bash-completion/completions/ RENAME gwhisper)
install(FILES complete.fish DESTINATION /usr/share/fish/vendor_completions.d/ RENAME gwhisper.fish)
