#!/bin/bash
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

SOURCE_DIR="$PWD"
BUILD_DIR="$SOURCE_DIR/build"
if [ ! -f "$BUILD_DIR/Makefile" ]; then
    mkdir $BUILD_DIR
    cd $BUILD_DIR
    cmake $SOURCE_DIR "$@" -DCMAKE_BUILD_TYPE=Debug 
    RC=$?
    if [ "$RC" -ne 0 ]; then
        # cmake failed
        exit $RC
    fi
    cd $SOURCE_DIR
fi
cd $BUILD_DIR
make -j$(nproc)
exit $?
