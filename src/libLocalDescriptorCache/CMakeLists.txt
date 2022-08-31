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
set(TARGET_SRC
    DescDbProxy.cpp
    )

add_protobuf_grpc_lib( ${TARGET_NAME}_protobuf PROTO_SOURCES LocalDescDb.proto)

add_library(${TARGET_NAME} ${TARGET_SRC} ${PROTO_SRC})

target_link_libraries(${TARGET_NAME}
    PUBLIC
    ${TARGET_NAME}_protobuf
     
    PRIVATE
    libGwhisperSettings
    gwhisperUtils
    version
    reflection
    cli
    )

