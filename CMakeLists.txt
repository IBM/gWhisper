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

cmake_minimum_required (VERSION 3.11)
project (gWhisper)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

################################################################################
# BUILD TYPE AND COMPILER FLAGS
################################################################################
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
# BUILD OPTIONS
################################################################################
option(GWHISPER_BUILD_TESTSERVER "Build a test server, which may be used to play around with gWhisper. It is also used by automated functional tests." ON)
option(GWHISPER_BUILD_TESTS "Build unit and functional tests - needs active internet connection for googletest" OFF)
option(GWHISPER_FORCE_BUILDING_GRPC "Fetch, build and statically link against gRPC even if gRPC is found to be installed" OFF)
option(GWHISPER_USE_BOOST_REGEX "Use Boost regex library instead of c++ standard library regex" OFF)

################################################################################
# MISC PROJECT SETUP
################################################################################
if(GWHISPER_BUILD_TESTS)
    enable_testing()
    message("Testing Enabled: Building unit and functional tests")
    message("                 Need access to github to be successful")
else()
    message("Testing Disabled")
endif()

include(gRPC)

if(GWHISPER_USE_BOOST_REGEX)
    add_definitions(-DBUILD_CONFIG_USE_BOOST_REGEX)
endif()

# this causes all built executables to be on build directory toplevel.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} )

# avoid to compile again when make install
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)


################################################################################
# GWHISPER CODE COMPONENTS
################################################################################
add_subdirectory(third_party)
add_subdirectory(src)
add_subdirectory(tests)

################################################################################
# INSTALL RULES FOR TAB-COMPLETION
################################################################################
install(FILES complete.bash DESTINATION /usr/share/bash-completion/completions/ RENAME gwhisper)
install(FILES complete.fish DESTINATION /usr/share/fish/vendor_completions.d/ RENAME gwhisper.fish)

# Generate new Key-Cert Pairs every build.  
# There is a bug regarding multiple output files in CMAKE 2.8.12.1 which is fixed in CMAKE 3.2 (https://cmake.org/Bug/view.php?id=15116).
add_custom_command(
OUTPUT cert-key-pair/ca.crt cert-key-pair/ca.key cert-key-pair/ca.srl cert-key-pair/client_crt.pem cert-key-pair/client_csr.pem cert-key-pair/client_key.pem cert-key-pair/server_crt.pem cert-key-pair/server_csr.pem cert-key-pair/server_key.pem
COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/cert-key-gen.sh ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(
MakeKeyCert
DEPENDS cert-key-pair/ca.crt cert-key-pair/ca.key cert-key-pair/ca.srl cert-key-pair/client_crt.pem cert-key-pair/client_csr.pem cert-key-pair/client_key.pem cert-key-pair/server_crt.pem cert-key-pair/server_csr.pem cert-key-pair/server_key.pem
)

