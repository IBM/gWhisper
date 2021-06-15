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


add_subdirectory(protoDoc)
add_subdirectory(libArgParse)
add_subdirectory(libCli)
add_subdirectory(gwhisper)
add_subdirectory(version)

get_filename_component(BIN_DIR_ABS ${CMAKE_BINARY_DIR}/. DIRECTORY)
get_filename_component(SRC_DIR_ABS ${CMAKE_SOURCE_DIR}/. DIRECTORY)

if( ${BIN_DIR_ABS} STREQUAL ${SRC_DIR_ABS} )
  add_custom_target(compileCommands
    # NOOP, the file is already at its destination
    )
else()
  add_custom_target(compileCommands
    COMMAND cp ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/compile_commands.json
    )
endif()
  

