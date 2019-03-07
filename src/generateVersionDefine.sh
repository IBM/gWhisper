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

# get build date
DATE=`date +%Y-%m-%d_%H:%M:%S`


if [ -z "`command -v git`" ]; then
    echo "$GWHISPER_BUILD_VERSION Build date: $DATE Build pwd: $PWD"
    exit 0
fi

git status &> /dev/null
if [ $? -ne 0 ]; then
    echo "$GWHISPER_BUILD_VERSION Build date: $DATE Build pwd: $PWD"
    exit 0
fi

# get unique readable commit identification
gitId=$(git describe --tags)

# check for uncommited changes
uncommitedChanges=""
git diff-index --quiet HEAD --
if [ $? -eq 0 ]; then
    uncommitedChanges="NoUncommitedChanges"
else
    uncommitedChanges="UncommitedChanges"
fi

# check for untracked files present
untrackedFiles=""
tmpUntracked=$(git ls-files --other --directory --exclude-standard)
if [ -z "$tmpUntracked" ]; then
    untrackedFiles="NoUntrackedFiles"
else
    untrackedFiles="UntrackedFiles"
fi

versionString="$GWHISPER_BUILD_VERSION Git identifier: ${gitId} Build date: ${DATE} ${uncommitedChanges} ${untrackedFiles}"

echo "#pragma once"
echo "#define GWHISPER_BUILD_VERSION \"$versionString\""
