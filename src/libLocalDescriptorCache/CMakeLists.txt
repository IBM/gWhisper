# Copyright 2021 IBM Corporation
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


set(TARGET_NAME "DescDbProxy")
# set(CMAKE_BUILD_TYPE Debug)
set(TARGET_SRC
    DescDbProxy.cpp
    )

add_protobuf_grpc_lib( ${TARGET_NAME}_protobuf PROTO_SOURCES LocalDescDb.proto)

add_library(${TARGET_NAME} ${TARGET_SRC} ${PROTO_SRC})

set_target_properties(
    ${TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )


ADD_DEPENDENCIES(${TARGET_NAME}

MakeKeyCert
)

target_link_libraries(${TARGET_NAME}
    PUBLIC
    ${TARGET_NAME}_protobuf

    # we need to link the whole reflection lib, even though the compiler might
    # think we do not use any symbols. (This is because gRPC dynamically
    # checks if reflection is linked and has no hard dependency on any symbols
    # there)
    # This is done with --no-as-needed in case we link dynamically and
    # --whole-archive in case we link statically.
    #-Wl,--push-state,--no-as-needed,--whole-archive ${LIB_GRPC++_REFLECTION} -Wl,--pop-state

    gwhisperUtils
    reflection
    
    PRIVATE
    version
    )

