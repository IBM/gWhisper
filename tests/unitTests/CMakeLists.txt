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


include_directories("${PROJECT_BINARY_DIR}")

set(TARGET_NAME "gwhisper_tests")
set(TARGET_SRC
    FixedStringTest.cpp
    ConcatenationTest.cpp
    AlternationTest.cpp
    OptionalTest.cpp
    RepetitionTest.cpp
    GrammarComboTests.cpp
    ParsedDocumentTest.cpp
    EscapedStringTest.cpp
    GrammarFactoryTest.cpp
    GwhisperUtilsTest.cpp
    MessageFormatterCustomTest.cpp
    testmain.cpp
    )

add_executable(${TARGET_NAME} ${TARGET_SRC})
add_dependencies(${TARGET_NAME} testServer_protobuf)

target_link_libraries (${TARGET_NAME}
    ArgParse
    gwhisperUtils
    reflection
    cli
    gtest
    $<$<BOOL:${GWHISPER_ENABLE_COVERAGE}>:coverage_config>
    )
if(APPLE)
    target_link_libraries(${TARGET_NAME} -Wl,-force_load,$<TARGET_FILE:testServer_protobuf>)
else()
    target_link_libraries(${TARGET_NAME} -Wl,--whole-archive,$<TARGET_FILE:testServer_protobuf>,--no-whole-archive)
endif()
if(BUILD_CONFIG_USE_BOOST_REGEX)
    target_link_libraries (${TARGET_NAME}
        boost_regex
    )
endif()

add_test(NAME UnitTests COMMAND ${TARGET_NAME})

if(GWHISPER_ENABLE_COVERAGE)
    gwhisper_coverage_target(
        TEST_BINARY ${TARGET_NAME}
        SOURCE_DIRS ${CMAKE_SOURCE_DIR}/src
    )
    gwhisper_coverage_full_target(
        UNIT_TEST_BINARY  ${TARGET_NAME}
        GWHISPER_BINARY   gwhisper
        TESTSERVER_BINARY testServer
        SOURCE_DIRS       ${CMAKE_SOURCE_DIR}/src
    )
endif()
