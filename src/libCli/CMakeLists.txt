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


set(TARGET_NAME "cli")
set(TARGET_SRC
    ./MessageParsing.cpp
    ./OutputFormatting.cpp
    ./GrammarConstruction.cpp
    ./Completion.cpp
    ./Call.cpp
    ./cliUtils.cpp
    )
add_library(${TARGET_NAME} ${TARGET_SRC})

# need to link against gpr to be able to use GPR in cliUtils

target_link_libraries ( ${TARGET_NAME}
    reflection
    ArgParse
    protoDoc
    grpc++
    )

if(BUILD_CONFIG_USE_BOOST_REGEX)
    target_link_libraries (${TARGET_NAME}
        boost_regex
    )
endif()
