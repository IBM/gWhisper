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

# avoid to compile again when make install
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY TRUE)

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_BINARY_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/src")

add_subdirectory(src)

add_subdirectory(tests)

add_subdirectory(third_party)

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

