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

if(GWHISPER_BUILD_TESTS)
    message("Building unit and functional tests")
    include(FetchContent)
    FetchContent_Declare (
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.11.0
        )
    FetchContent_GetProperties(googletest)
    if(NOT googletest_POPULATED)
        message("Downloading googletest...")
        FetchContent_Populate(googletest)
        message("Download of googletest finished.")
    endif()
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)

    add_subdirectory(unitTests)
    add_subdirectory(functionTests)
else()
    message("Not building unit and functional tests")
endif()

if(GWHISPER_BUILD_TESTSERVER OR GWHISPER_BUILD_TESTS)
    message("Building testserver")
    add_subdirectory(testServer)
endif()
