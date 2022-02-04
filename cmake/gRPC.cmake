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
if(NOT GWHISPER_FORCE_BUILDING_GRPC)
    # find grpc + protobuf libs and code generators:
    find_library(LIB_PROTOBUF protobuf)
    find_library(LIB_GRPC grpc)
    find_library(LIB_GRPC++ grpc++)
    find_library(LIB_GRPC++_REFLECTION grpc++_reflection)
    find_library(LIB_GPR gpr)
    find_program (PROTOC protoc)
    find_program (PROTOC_GRPC_PLUGIN grpc_cpp_plugin)

    if(
            LIB_PROTOBUF STREQUAL "LIB_PROTOBUF-NOTFOUND"
            OR
            LIB_GRPC                STREQUAL "LIB_GRPC-NOTFOUND"
            OR
            LIB_GRPC++              STREQUAL "LIB_GRPC++-NOTFOUND"
            OR
            LIB_GRPC++_REFLECTION   STREQUAL "LIB_GRPC++_REFLECTION-NOTFOUND"
            OR
            LIB_GPR                 STREQUAL "LIB_GPR-NOTFOUND"
            OR
            PROTOC                  STREQUAL "PROTOC-NOTFOUND"
            OR
            PROTOC_GRPC_PLUGIN      STREQUAL "PROTOC_GRPC_PLUGIN-NOTFOUND"
      )
        message("No gRPC installation found on this system.")
        set(GRPC_NOT_FOUND TRUE)
    else()
        message("gRPC installation found on the system, using this. If you do not want this and instead want CMake to download, build and statically link against gRPC, use '-D GWHISPER_FORCE_BUILDING_GRPC=ON'")
    endif()
endif()

if(GWHISPER_FORCE_BUILDING_GRPC OR GRPC_NOT_FOUND)
    message("Using gRPC built from source")

    # Try to disable as much testing in gRPC as possible:
    set(RE2_BUILD_TESTING CACHE BOOL OFF FORCE)

    unset(LIB_PROTOBUF CACHE)
    unset(LIB_GRPC CACHE)
    unset(LIB_GRPC++ CACHE)
    unset(LIB_GRPC++_REFLECTION CACHE)
    unset(LIB_GPR CACHE)
    unset(PROTOC CACHE)
    unset(PROTC_GRPC_PLUGIN CACHE)

    include(FetchContent)
    FetchContent_Declare(
        grpc
        GIT_REPOSITORY https://github.com/grpc/grpc
        GIT_TAG        v1.43.0
        GIT_PROGRESS TRUE
    )
    set(FETCHCONTENT_QUIET OFF)
    FetchContent_GetProperties(grpc)
    if(NOT grpc_POPULATED)
        message("Downloading gRPC and its dependencies. This might take a while...")
        FetchContent_Populate(grpc)
        message("Download of gRPC finished")
    endif()
    # Adding the "EXCLUDE_FROM_ALL" here, to prevent install targets to be added
    # As we do not want to install whole gRPC and its depdendencies to the system.
    # We do not want to build any tests from grpc
    set(BUILD_TESTING OFF)

    # The zlib module is not intended to be included as sub directory
    # Building tests without the neded dependencies, making
    # a regression run fail
    # zlib can be expected to be systemwide available
    set(gRPC_ZLIB_PROVIDER "package" CACHE STRING "force overwritten by gWhisper" FORCE )

    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "s390x")
        set(gRPC_SSL_PROVIDER "package" CACHE STRING "force overwritten by gWhisper, as boringSSL does not support s390x -> need to fall-back to system installed libssl" FORCE )
    endif()

    add_subdirectory(${grpc_SOURCE_DIR} ${grpc_BINARY_DIR} EXCLUDE_FROM_ALL)

    # Since FetchContent uses add_subdirectory under the hood, we can use
    # the grpc targets directly from this build.
    set(LIB_PROTOBUF "libprotobuf" CACHE STRING "Libprotobuf" FORCE )
    set(LIB_GRPC "grpc" CACHE STRING "grpc"  FORCE)
    set(LIB_GRPC++ "grpc++" CACHE STRING "grpc++" FORCE)
    set(LIB_GRPC++_REFLECTION "grpc++_reflection" CACHE STRING "grpc++_reflection" FORCE)
    set(PROTOC "$<TARGET_FILE:protoc>" CACHE STRING "Protoc" FORCE)
    set(PROTOC_GRPC_PLUGIN "$<TARGET_FILE:grpc_cpp_plugin>" CACHE STRING "Protoc GRPC PLUGIN" FORCE)

    get_target_property( _PROTOBUF_INCLUDE_DIR libprotobuf INTERFACE_INCLUDE_DIRECTORIES)
    set(PROTOBUF_INCLUDE_DIR ${_PROTOBUF_INCLUDE_DIR} CACHE STRING "protobuf includes" FORCE)
endif()

function (_generate_protobuf_source _proto_sources _includes _cpp_headers _cpp_sources)
    string(REPLACE ".proto" ".pb.cc" _sources ${_proto_sources})
    string(REPLACE ".proto" ".pb.h" _headers ${_proto_sources})
    set(${_cpp_headers} ${${_cpp_headers}} ${_headers} PARENT_SCOPE)
    set(${_cpp_sources} ${${_cpp_sources}} ${_sources} PARENT_SCOPE)
    list(APPEND _includes ${CMAKE_CURRENT_SOURCE_DIR} ${PROTOBUF_INCLUDE_DIR})
    list(REMOVE_DUPLICATES _includes)
    set(_include_commands $<$<BOOL:${_includes}>:-I$<JOIN:${_includes}, -I>>)
    add_custom_command(
        OUTPUT  ${_sources} ${_headers}
        COMMAND ${PROTOC} "${_include_commands}" --cpp_out=${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${_proto_sources}
        DEPENDS ${_proto_sources}
        COMMAND_EXPAND_LISTS
    )
endfunction()

function (_generate_protobuf_grpc_source _proto_source _includes _cpp_headers _cpp_sources)
    string(REPLACE ".proto" ".grpc.pb.cc" _sources ${_proto_source})
    string(REPLACE ".proto" ".grpc.pb.h" _headers ${_proto_source})
    set(${_cpp_headers} ${${_cpp_headers}} ${_headers} PARENT_SCOPE)
    set(${_cpp_sources} ${${_cpp_sources}} ${_sources} PARENT_SCOPE)
    list(APPEND _includes ${CMAKE_CURRENT_SOURCE_DIR} ${PROTOBUF_INCLUDE_DIR})
    list(REMOVE_DUPLICATES _includes)
    # TODO: I uncommented this, do not really understand whay this does,
    #       Does not seem to be used anywhere.
    #file(GENERATE OUTPUT debug.txt CONTENT "$<$<BOOL:${_includes}>:-I$<JOIN:${_includes}, -I>>")
    #file(GENERATE OUTPUT debug2.txt CONTENT "$<$<BOOL:$<GENEX_EVAL:${_includes}>>:-I$<JOIN:${_includes}, -I>>")
    set(_include_commands $<$<BOOL:${_includes}>:-I$<JOIN:${_includes}, -I>>)

    add_custom_command(
        OUTPUT  ${_sources} ${_headers}
        COMMAND ${PROTOC} "${_include_commands}" --grpc_out=${CMAKE_CURRENT_BINARY_DIR} --plugin=protoc-gen-grpc=${PROTOC_GRPC_PLUGIN} ${CMAKE_CURRENT_SOURCE_DIR}/${_proto_source}
        DEPENDS ${_proto_source}
        COMMAND_EXPAND_LISTS
    )
endfunction()

function(add_protobuf_grpc_lib _lib_name  )
    set(multiValueArgs PROTO_SOURCES LINK_PROTO_LIB)
    cmake_parse_arguments( add_protobuf_grpc_lib "" "" "${multiValueArgs}" ${ARGN})
    if( "" STREQUAL "${add_protobuf_grpc_lib_PROTO_SOURCES}")
        MESSAGE(FATAL_ERROR "add_protobuf_grpc_lib called with empty PROTO_SOURCES")
    endif()
    foreach( _consumed_proto_lib IN LISTS add_protobuf_grpc_lib_LINK_PROTO_LIB)
        get_target_property(include_path ${_consumed_proto_lib} _proto_include)
        list(APPEND _all_consumed_proto_include_dirs ${include_path})
    endforeach()

    foreach( _proto_source IN LISTS add_protobuf_grpc_lib_PROTO_SOURCES)
        _generate_protobuf_source(${_proto_source} "${_all_consumed_proto_include_dirs}" ${_lib_name}_HEADERS ${_lib_name}_SOURCES )
        _generate_protobuf_grpc_source(${_proto_source} "${_all_consumed_proto_include_dirs}" ${_lib_name}_HEADERS  ${_lib_name}_SOURCES)
    endforeach()

    add_library( ${_lib_name} ${${_lib_name}_SOURCES} ${${_lib_name}_HEADERS})

    set_target_properties(${_lib_name} PROPERTIES _proto_include ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries( ${_lib_name}
        PUBLIC
        ${LIB_PROTOBUF} pthread
        ${LIB_GRPC++}
        ${LIB_GRPC}
        PRIVATE
        ${add_protobuf_grpc_lib_LINK_PROTO_LIB}
        ${LIB_GPR}
        )
    target_include_directories(${_lib_name}
        PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}
        PRIVATE
        ${PROTOBUF_INCLUDE_DIR}
    )
endfunction()
