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

set(TARGET_NAME "reflection")
set(TARGET_SRC
    proto_reflection_descriptor_database.cc
    cli_call.cc
    )

# find grpc + protobuf libs and code generators:
find_library(LIB_PROTOBUF protobuf)
find_library(LIB_GRPC grpc)
find_library(LIB_GRPC++ grpc++)
find_library(LIB_GRPC++_reflection grpc++_reflection)
find_program (PROTOC protoc)
find_program (PROTOC_GRPC_PLUGIN grpc_cpp_plugin)
set(GRPC_LIBS_REFLECTION -Wl,--no-as-needed ${LIB_GRPC++_reflection} -Wl,--as-needed ${LIB_GRPC++} ${LIB_GRPC} ${LIB_PROTOBUF})
message(STATUS "PROTOC = ${PROTOC}")
message(STATUS "PROTOC_GRPC_PLUGIN = ${PROTOC_GRPC_PLUGIN}")
message(STATUS "DYNAMIC GRPC LINKING INFO = ${GRPC_LIBS_REFLECTION}")

# determine proto file source and binary directories (binary directory used to
# write generated code to)
#set(PROTO_FILE_BASE_SRC_PATH ${CMAKE_SOURCE_DIR}/third_party)
set(PROTO_FILE_BASE_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR})
string(REGEX REPLACE "^${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" PROTO_FILE_BASE_DST_PATH ${PROTO_FILE_BASE_SRC_PATH})
message(STATUS "PROTO_FILE_BASE_SRC_PATH = ${PROTO_FILE_BASE_SRC_PATH}")
message(STATUS "PROTO_FILE_BASE_DST_PATH = ${PROTO_FILE_BASE_DST_PATH}")

# add reflection generated code location to include path:
include_directories(${PROTO_FILE_BASE_DST_PATH})

# add rules for code generation:
add_custom_command(
    OUTPUT reflection.pb.cc reflection.pb.hh
    COMMAND ${PROTOC} -I${PROTO_FILE_BASE_SRC_PATH} --cpp_out=${PROTO_FILE_BASE_DST_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/reflection.proto
    DEPENDS reflection.proto
    )
add_custom_command(
    OUTPUT reflection.grpc.pb.cc reflection.grpc.pb.hh
    COMMAND ${PROTOC} -I${PROTO_FILE_BASE_SRC_PATH} --grpc_out=${PROTO_FILE_BASE_DST_PATH} --plugin=protoc-gen-grpc=${PROTOC_GRPC_PLUGIN} ${CMAKE_CURRENT_SOURCE_DIR}/reflection.proto
    DEPENDS reflection.proto
    )

add_library(${TARGET_NAME} ${TARGET_SRC}
    # NOTE: it is very important to list the headers here and not as dependencies
    #       otherwise with CMAKE 2.8 we code generation will not be triggered
    reflection.pb.hh
    reflection.grpc.pb.hh
    )

# NOTE: we only need the reflection.proto generated headers here, and do not need
#       to link against generated source files, as grpc libs seem to already
#       bring the reflection lib
target_link_libraries(${TARGET_NAME}
    ${GRPC_LIBS_REFLECTION}
    )
