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

include_guard()
if(USE_PRE_INSTALLED)
    # find grpc + protobuf libs and code generators:
    find_library(LIB_PROTOBUF protobuf)
    find_library(LIB_GRPC grpc)
    find_library(LIB_GRPC++ grpc++)
    find_library(LIB_GRPC++_reflection grpc++_reflection)
    find_program (PROTOC protoc)
    find_program (PROTOC_GRPC_PLUGIN grpc_cpp_plugin)
else()
    include(FetchContent)
    FetchContent_Declare(
        gRPC
        GIT_REPOSITORY https://github.com/grpc/grpc
        GIT_TAG        v1.32.0  # e.g v1.28.0
    )
    set(FETCHCONTENT_QUIET OFF)
    FetchContent_MakeAvailable(gRPC)
    # Since FetchContent uses add_subdirectory under the hood, we can use
    # the grpc targets directly from this build.
    set(LIB_PROTOBUF "libprotobuf" CACHE STRING "Libprotobuf" FORCE)
    set(LIB_GRPC "grpc" CACHE STRING "grpc" FORCE)
    set(LIB_GRPC++ "grpc++" CACHE STRING "grpc++" FORCE)
    set(LIB_GRPC++_REFLECTION "grpc++_reflection" CACHE STRING "grpc++_reflection" FORCE)
    set(PROTOC "$<TARGET_FILE:protoc>" CACHE STRING "Protoc" FORCE)
    set(PROTOC_GRPC_PLUGIN "$<TARGET_FILE:grpc_cpp_plugin>" CACHE STRING "Protoc GRPC PLUGIN" FORCE)
    message(PROTOC_GRPC_PLUGIN ${PROTOC_GRPC_PLUGIN})
    message(PROTOC ${PROTOC})

    get_target_property( _PROTOBUF_INCLUDE_DIR libprotobuf INTERFACE_INCLUDE_DIRECTORIES)
    set(PROTOBUF_INCLUDE_DIR ${_PROTOBUF_INCLUDE_DIR} CACHE STRING "protobuf includes" FORCE)
    if(CMAKE_CROSSCOMPILING)
        find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
    else()
        set(_GRPC_CPP_PLUGIN_EXECUTABLE "$<TARGET_FILE:grpc_cpp_plugin>" CACHE STRING "grpc cpp plugin")
    endif()
endif()

function (_generate_protobuf_sources _proto_sources _cpp_headers _cpp_sources)
    string(REPLACE ".proto" ".pb.cc" _sources ${_proto_sources})
    string(REPLACE ".proto" ".pb.h" _headers ${_proto_sources})
    set(${_cpp_headers} ${${_cpp_headers}} ${_headers} PARENT_SCOPE)
    set(${_cpp_sources} ${${_cpp_sources}} ${_sources} PARENT_SCOPE)
    add_custom_command(
        OUTPUT  ${_sources} ${_headers}
        COMMAND ${PROTOC} -I${CMAKE_CURRENT_SOURCE_DIR} -I${PROTOBUF_INCLUDE_DIR} --cpp_out=${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${_proto_sources}
        DEPENDS ${_proto_sources}
    )
endfunction()

function (_generate_protobuf_grpc_sources _proto_sources _cpp_headers _cpp_sources)
    string(REPLACE ".proto" ".grpc.pb.cc" _sources ${_proto_sources})
    string(REPLACE ".proto" ".grpc.pb.h" _headers ${_proto_sources})
    set(${_cpp_headers} ${${_cpp_headers}} ${_headers} PARENT_SCOPE)
    set(${_cpp_sources} ${${_cpp_sources}} ${_sources} PARENT_SCOPE)
    add_custom_command(
        OUTPUT  ${_sources} ${_headers}
        COMMAND ${PROTOC} -I${CMAKE_CURRENT_SOURCE_DIR} -I${PROTOBUF_INCLUDE_DIR} --grpc_out=${CMAKE_CURRENT_BINARY_DIR} --plugin=protoc-gen-grpc=${PROTOC_GRPC_PLUGIN} ${CMAKE_CURRENT_SOURCE_DIR}/${_proto_sources}
        DEPENDS ${_proto_sources}
    )
endfunction()

function(add_protobuf_lib _lib_name _proto_sources _include )
    _generate_protobuf_sources(${_proto_sources} ${_lib_name}_HEADERS ${_lib_name}_SOURCES)
    add_library( ${_lib_name} ${${_lib_name}_SOURCES} ${${_lib_name}_HEADERS})

    target_link_libraries( ${_lib_name}
        PUBLIC
        grpc++_reflection
        libprotobuf
        )
    target_include_directories(${_lib_name}
        PUBLIC
        ${PROTOBUF_INCLUDE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        )
endfunction()

function(add_protobuf_grpc_lib _lib_name _proto_sources )
    _generate_protobuf_sources(${_proto_sources} ${_lib_name}_HEADERS ${_lib_name}_SOURCES _include)
    _generate_protobuf_grpc_sources(${_proto_sources} ${_lib_name}_HEADERS ${_lib_name}_SOURCES)
    add_library( ${_lib_name} ${${_lib_name}_SOURCES} ${${_lib_name}_HEADERS})

    target_link_libraries( ${_lib_name}
        PUBLIC
        grpc++_reflection
        libprotobuf
        )
    target_include_directories(${_lib_name}
        PUBLIC
        ${PROTOBUF_INCLUDE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        )
endfunction()