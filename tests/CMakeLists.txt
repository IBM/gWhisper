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

if(EXISTS "${PROJECT_SOURCE_DIR}/third_party/googletest/CMakeLists.txt")
    # we only build tests if googletest is available
    # If the gWhisper source package is downloaded from GitHub as archive (tar or zip)
    # submodules are not included. This ensures, that we build at least the functional
    # code in this case.
  add_subdirectory(unitTests)
  add_subdirectory(functionTests)
else()
    message(WARNING "googletest submodule not found, not building tests. Please be sure to get the submodules with 'git submodule update --init' to also build tests.")
endif()
add_subdirectory(testServer)
