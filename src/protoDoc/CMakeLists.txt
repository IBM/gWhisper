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

set(TARGET_NAME "protoDoc")
set(TARGET_SRC
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
#message(STATUS "DYNAMIC GRPC LINKING INFO = ${GRPC_LIBS_REFLECTION}")

# determine proto file source and binary directories (binary directory used to
# write generated code to)
set(PROTO_FILE_BASE_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR})
string(REGEX REPLACE "^${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" PROTO_FILE_BASE_DST_PATH ${PROTO_FILE_BASE_SRC_PATH})
message(STATUS "PROTO_FILE_BASE_SRC_PATH = ${PROTO_FILE_BASE_SRC_PATH}")
message(STATUS "PROTO_FILE_BASE_DST_PATH = ${PROTO_FILE_BASE_DST_PATH}")


# add reflection generated code location to include path:
include_directories(${PROTO_FILE_BASE_DST_PATH})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_BINARY_DIR})


# add rules for code generation:
add_custom_command(
    OUTPUT protoDoc.pb.cc protoDoc.pb.h
    COMMAND ${PROTOC} --cpp_out=${PROTO_FILE_BASE_DST_PATH} -I ${PROTO_FILE_BASE_SRC_PATH} ${PROTO_FILE_BASE_SRC_PATH}/protoDoc.proto
    DEPENDS protoDoc.proto
    )
add_custom_command(
    OUTPUT protoDoc.grpc.pb.cc protoDoc.grpc.pb.h
    COMMAND ${PROTOC} --grpc_out=${PROTO_FILE_BASE_DST_PATH} -I ${PROTO_FILE_BASE_SRC_PATH} ${PROTO_FILE_BASE_SRC_PATH}/protoDoc.proto --plugin=protoc-gen-grpc=${PROTOC_GRPC_PLUGIN}
    DEPENDS protoDoc.proto
    )

set(PROTO_SRC protoDoc.pb.cc protoDoc.grpc.pb.cc)

add_executable(${TARGET_NAME} ${TARGET_SRC} ${PROTO_SRC})
target_link_libraries(${TARGET_NAME}
    ${GRPC_LIBS_REFLECTION}
    )