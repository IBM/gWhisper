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

# This is a code-generator which attempts to generate a header file containing
# version string. Following data sources are tried to get version information:
# 1. git tags, commit hashes and status
# 2. A version string in the environment variable GWHISPER_BUILD_VERSION if set.
# 3. Build date and time
# 4. Hard-coded version string HARD_CODED_VERSION found in this file.
#
# As an optional argument to this script a path containing the .git directory
# for the gWhisper repository can be provided. This prevents git from searching
# the FS tree for a git repository in case gWhisper is sub-packaged into another
# repository

# TODO: we could do this via git hooks. Problem is just that hooks are not
#       really part of the repository. I.e. each user would need to install the
#       hooks. Will need to find some solution
HARD_CODED_VERSION="v3.3.1"

# get build date
DATE=`date +%Y-%m-%d_%H:%M:%S`

GITDIR=$1

if [ "$GITDIR" = "" ]; then
    GITCMD="git"
else
    cd $GITDIR
    GITCMD="git --git-dir=.git"
fi

if [ "$GWHISPER_BUILD_VERSION" != "" ]; then
    GWHISPER_BUILD_VERSION="Environment version:$GWHISPER_BUILD_VERSION "
fi

if [ -z "`command -v git`" ]; then
    versionString="${GWHISPER_BUILD_VERSION}Hard-coded version: $HARD_CODED_VERSION Build date: $DATE"
fi

$GITCMD status &> /dev/null
if [ $? -ne 0 ]; then
    versionString="${GWHISPER_BUILD_VERSION}Hard-coded version: $HARD_CODED_VERSION Build date: $DATE"
fi

if [ "$versionString" = "" ]; then
    # version string is still empty -> we can use git to generate the version string

    # get unique readable commit identification
    gitId=$($GITCMD describe --tags)

    # check for uncommited changes
    uncommitedChanges=""
    $GITCMD diff-index --quiet HEAD --
    if [ $? -eq 0 ]; then
        uncommitedChanges="NoUncommitedChanges"
    else
        uncommitedChanges="UncommitedChanges"
    fi

    # check for untracked files present
    untrackedFiles=""
    #tmpUntracked=$($GITCMD ls-files --other --directory --exclude-standard)
    tmpUntracked=$($GITCMD status | grep "Untracked files")
    if [ "$tmpUntracked" = "" ]; then
        untrackedFiles="NoUntrackedFiles"
    else
        untrackedFiles="UntrackedFiles"
    fi

    versionString="${GWHISPER_BUILD_VERSION}Git identifier: ${gitId} Build date: ${DATE} ${uncommitedChanges} ${untrackedFiles}"
fi

echo "#pragma once"
echo "#define GWHISPER_BUILD_VERSION \"$versionString\""
