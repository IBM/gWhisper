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

set(TARGET_NAME "reflection")
set(TARGET_SRC
    gRPC_utils/proto_reflection_descriptor_database.h
    gRPC_utils/cli_call.h
    cli_call.cc
    proto_reflection_descriptor_database.cc
    )


add_protobuf_grpc_lib( ${TARGET_NAME}_protobuf PROTO_SOURCES reflection.proto )

add_library( ${TARGET_NAME} ${TARGET_SRC})
target_link_libraries(${TARGET_NAME} PUBLIC ${TARGET_NAME}_protobuf)

target_include_directories(${TARGET_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} PRIVATE gRPC_utils)

