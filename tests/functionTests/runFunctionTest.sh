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

# this is a very simple testcase executing script.
# it receives two mandatory arguments:
# 1. the path to the build directory of gWhisper
# 2. a file containing test cases
# the script will start a grpc test server against wich tests can be run.
# All test cases described in the given file are executed.
# If one or more fails the script exits with return code 1. otherwise with 0.
#
# The test case file is formatted as follows:
# After a line beginning with "#START_TEST" followed by a space and a test case description
# a SINGLE line containing command is expected which will be executed in a sub-shell.
# If this line contains the string "@@CMD@@" it will be replaced with a path to
# the gwhisper executable.
# If this line contains the string "@@PTB@@" it will be replaced with a path to 
# the gwhisper build directory
# All following lines until a line Starting with "#END_TEST" are the expected command
# output.
# If one of those lines starts with a "/" the line is interpreted as a regex.
# If one of those lines starts with a "?" the line is interpreted as a regex,
# but is optional. This means, if the regex does not match, the next expected
# line is tried.


# cli arguments
build=$1
gwhisper=$build/gwhisper
testFile=$2
testResources=$build/tests/functionTests/resources

# colors
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

# starting test server
cd $build
echo "Starting server: $build/testServer ...";
$build/testServer &
serverPID=$!
sleep 0.2

echo "Running Completion tests..."

# state machine for parser
state="FIND_TEST"
pathToBuild=""
cmd=""
received=()
expected=()
curline=0
testline=0
testname=""
failedTests=()
numTests=0

# parse testcase file and execute tests
while IFS='' read -r line || [[ -n "$line" ]]; do
    ((curline=curline+1))
    # echo "Text read from file: $line"

    if [[ "$line" =~ ^#START_TEST ]] && [[ $state = "FIND_TEST" ]]; then
        testline=$curline
        testname=${line:12}
        state="PARSE_CMD"
        echo "#################################################################"
        echo "Executing test '$testname' at line $testline"
        continue
    fi
    if [[ "$line" =~ ^#END_TEST ]] && [[ $state = "PARSE_RESULT" ]]; then
        ((numTests=numTests+1))
        fail=false
        failtext=""
        idx=0
        for expectedLine in "${expected[@]}"
        do
            if [ ${#received[@]} -le $idx ]; then
                # prevent over reading the received arrwy
                if [ ${expectedLine:0:1} = "?" ]; then
                    continue;
                else
                    fail=true
                    failtext="line $(((idx+1))) expected more lines."
                    break;
                fi
            fi;
            if [ ${expectedLine:0:1} = "?" ]; then
                # optional regex
                expectedLine=${expectedLine:1}
                if ! [[ ${received[$idx]} =~ $expectedLine ]]; then
                    continue
                fi
            elif [ ${expectedLine:0:1} = "/" ]; then
                # mandatory regex
                expectedLine=${expectedLine:1}
                if ! [[ ${received[$idx]} =~ $expectedLine ]]; then
                    fail=true
                    failtext="line $(((idx+1))) received text '${received[$idx]}' does not match expected regex '$expectedLine'."
                    break
                fi
            else
                # mandatory exact match
                if [ "$expectedLine" != "${received[$idx]}" ]; then
                    fail=true
                    failtext="line $(((idx+1))) received text '${received[$idx]}' and expected text '$expectedLine' does not match."
                    break
                fi
            fi
            ((idx=idx+1))
        done
        if [ ${#received[@]} -ne $idx ]; then
            fail=true
            failtext="expected number of lines ${idx} (of ${#expected[@]}) and received number of lines ${#received[@]} do not match!"
        fi;
        if [ $fail = true ]; then
            failedTests+=("line: ${testline}, name: '${testname}'")
            echo " Received:"
            printf '  '%s'\n' "${received[@]}"
            echo " Expected:"
            printf '  '%s'\n' "${expected[@]}"
            echo -e " ${RED}FAIL:${NC} $failtext"
        else
            echo -e " ${GREEN}OK${NC}"
        fi
        state="FIND_TEST"
        continue
    fi

    if [[ $state = "PARSE_CMD" ]]; then
        cmd=${line//@@CMD@@/$gwhisper}
        echo " resolve cmd '$cmd'"
        newLine=${cmd//@@PTB@@/$build}
        echo " execute new command '$newLine'"
        out=$(eval "$newLine 2>&1") # use eval here to correctly split args into arg array
        IFS=$'\n' received=($out)
        state="PARSE_RESULT"
        expected=()
        continue
    fi

    if [[ $state = "PARSE_RESULT" ]]; then
        expected+=("$line")
        continue
    fi

   
done < "$testFile"

if [[ $state != "FIND_TEST" ]]; then
    echo -e "${RED}File ended in the middle of a test case. Did you forget to add #END_TEST somewhere?${NC}"
    exit 1
fi

# analyze test result:
echo "#################################################################"
echo "#################################################################"
numFailed=${#failedTests[@]}
numSucceeded=$((numTests-numFailed))
echo "$numTests tests finished. Successful: $numSucceeded, Failed: $numFailed"
rc=0
if [ ${#failedTests[@]} -ne 0 ]; then
    echo -e "The following tests failed:"
    printf ' %s\n' "${failedTests[@]}"
    echo -e "${RED}$numFailed TESTS FAILED${NC}"
    rc=1
else
    echo -e "${GREEN}ALL $numTests TESTS SUCCEEDED${NC}"
fi

# stopping test server
echo "Stopping Server..."
kill $serverPID

# return test result
exit $rc
