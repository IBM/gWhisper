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


set(TARGET_NAME "gwhisper")

# this converts the help string into a raw string for use with --help
add_custom_command(
    OUTPUT HelpString.h
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/generateRawStringFile.sh ${CMAKE_SOURCE_DIR}/doc/Usage.txt ${CMAKE_CURRENT_BINARY_DIR}/HelpString.h
    DEPENDS ${CMAKE_SOURCE_DIR}/doc/Usage.txt
    )

set(TARGET_SRC
    gwhisper.cpp
    HelpString.h
    )
add_executable(${TARGET_NAME} ${TARGET_SRC})
target_link_libraries ( ${TARGET_NAME}
    cli
    reflection
    )

add_dependencies(${TARGET_NAME} compileCommands versionDefine)

install(TARGETS ${TARGET_NAME}
    RUNTIME DESTINATION bin
    )
