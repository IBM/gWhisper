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

add_test(NAME CompletionFunctionTests COMMAND ${PROJECT_SOURCE_DIR}/tests/functionTests/runFunctionTest.sh ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/tests/functionTests/completionTests.txt)

add_test(NAME CompletionFunctionTests_fish COMMAND ${PROJECT_SOURCE_DIR}/tests/functionTests/runFunctionTest.sh ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/tests/functionTests/completionTests_fish.txt)

add_test(NAME RpcExecutionTests COMMAND ${PROJECT_SOURCE_DIR}/tests/functionTests/runFunctionTest.sh ${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/tests/functionTests/rpcExecutionTests.txt)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources/data.bin DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/resources)
