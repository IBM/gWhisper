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

set(TARGET_NAME "testServer")
set(TARGET_SRC
    testServer.cpp
    ServiceScalarTypeRpcs.cpp
    ServiceNestedTypeRpcs.cpp
    ServiceStatusHandling.cpp
    ServiceStreamingRpcs.cpp
    ServiceComplexTypeRpcs.cpp
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
set(PROTO_FILE_BASE_SRC_PATH ${CMAKE_SOURCE_DIR})
#set(PROTO_FILE_BASE_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR})
string(REGEX REPLACE "^${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" PROTO_FILE_BASE_DST_PATH ${PROTO_FILE_BASE_SRC_PATH})
message(STATUS "PROTO_FILE_BASE_SRC_PATH = ${PROTO_FILE_BASE_SRC_PATH}")
message(STATUS "PROTO_FILE_BASE_DST_PATH = ${PROTO_FILE_BASE_DST_PATH}")

# add reflection generated code location to include path:
include_directories(${PROTO_FILE_BASE_DST_PATH})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_BINARY_DIR})

# FIXME: proto file compiling needs cleaning up!!
set(PROTO_DOC_SRC_PATH ${CMAKE_SOURCE_DIR}/src/protoDoc)
set(PROTO_DOC_BIN_PATH ${CMAKE_BINARY_DIR}/src/protoDoc)

include_directories(${PROTO_DOC_BIN_PATH})
message(STATUS "PROTO_DOC_SRC_PATH = ${PROTO_DOC_SRC_PATH}")
include_directories(${PROTO_DOC_BIN_PATH})
# add rules for code generation:
add_custom_command(
    OUTPUT examples.pb.cc examples.pb.h
    COMMAND ${PROTOC} -I${PROTO_FILE_BASE_SRC_PATH} -I ${PROTO_DOC_SRC_PATH} --cpp_out=${PROTO_FILE_BASE_DST_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/examples.proto
    DEPENDS examples.proto
    )
add_custom_command(
    OUTPUT examples.grpc.pb.cc examples.grpc.pb.h
    COMMAND ${PROTOC} -I${PROTO_FILE_BASE_SRC_PATH} -I ${PROTO_DOC_SRC_PATH} --grpc_out=${PROTO_FILE_BASE_DST_PATH} --plugin=protoc-gen-grpc=${PROTOC_GRPC_PLUGIN} ${CMAKE_CURRENT_SOURCE_DIR}/examples.proto
    DEPENDS examples.proto
    )
set(PROTO_SRC examples.pb.cc examples.grpc.pb.cc)

add_executable(${TARGET_NAME} ${TARGET_SRC} ${PROTO_SRC})

target_link_libraries(${TARGET_NAME}
    ${GRPC_LIBS_REFLECTION}
    protoDoc
    )

