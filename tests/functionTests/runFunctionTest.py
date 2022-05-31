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
# If a test case requires MULTIPLE independent commands, every command
# is to be nested between "#EXEC_CMD" and "#END_CMD"
# If a command line contains the string "@@CMD@@" it will be replaced with a path to
# the gwhisper executable.
# If this line contains the string "@@PTC@@" it will be replaced with a path to 
# the certificate directory
# All following lines until a line starting with "#END_TEST" or "#END_CMD" respectively
# are the expected command output.
# If one of those lines starts with a "/" the line is interpreted as a regex.
# If one of those lines starts with a "?" the line is interpreted as a regex,
# but is optional. This means, if the regex does not match, the next expected
# line is tried.
# 
import sys
import subprocess
import time
import re

# colors
def prRed(text): print("\033[91m {}\033[00m" .format(text))
def prGreen(text): print("\033[92m {}\033[00m" .format(text))

# cli arguments
gwhisper = sys.argv[1]
testServer = sys.argv[2]
testResources = sys.argv[3]
certs = sys.argv[4]
testFile = sys.argv[5]

# starting test server
print('Starting server: \'{} --certBasePath {} ...\''.format(testServer, certs))
startProcess = subprocess.Popen([testServer,"--certBasePath", certs], stdout=subprocess.PIPE)
#for out in startProcess.stdout:
#    print(out)
time.sleep(0.2)

print('Starting server: \'{}\' ...done'.format(testServer))
print('Running Completion tests...')

# state machine for parser
state = "FIND_TEST"
pathToBuild = ""
received = []
expected = []
curline = 0
testline = 0
testname = ""
failedTests = []
numTests = 0
multiCmd = False

# parse testcase file and execute tests
line = " "
with open(testFile) as file:
    while True:
        line  = file.readline()
        curline += 1
        if (line.startswith("#END_CMD")):
            parse_end = False

        if ((line.startswith("#START_TEST")) and (state == "FIND_TEST")):
            testline = curline
            testname = line[12:]
            testname = testname.replace("\n", "")
            state = "PARSE_CMD"
            print('#################################################################') 
            print ("Executing test '{}' at line {}".format(testname, testline))
            continue
        # end-if
        if ((line.startswith("#EXEC_CMD")) and (state == "PARSE_CMD")):
            multiCmd = True
            continue

        if ((line.startswith("#END_TEST") and state == "PARSE_RESULT") or (line.startswith("#END_CMD") and state == "PARSE_CMD_RESULT")):
            numTests += 1
            fail = False
            failtext = ""
            idx = 0

            for expectedLine in expected:
                if len(expected)>len(received):
                    fail = True
                    failtext = "Test '{}' at line {} received not enough lines.".format(testname, testline)
                    break
                if(len(expected) <= idx):
                    if(expectedLine.startswith("?")):
                        # prevent over reading the received array
                        continue
                    else:
                        fail = True
                        failtext = "line {} expected more lines.".format(idx+1)
                        break
                    # end-if
                # end-if
                if (expectedLine.startswith("?")):
                    # optional regex
                    expectedLine = expectedLine[1:].replace('\n','')
                    try:
                        regex = re.compile(expectedLine)
                    except re.error:
                        print(f'Invalid REGEX at line {idx+1}. Check your testfile.')
                        
                    match = regex.search(received[idx]) # match with incoming output
                    if (not match):
                        continue
                elif(expectedLine.startswith("/")):
                    # mandatory regex
                    expectedLine = expectedLine[1:].replace('\n','')
                    try:
                        regex = re.compile(expectedLine) # regex is not escaped
                    except re.error:
                        print(f'Invalid REGEX at line {curline}. Check your testfile.')

                    match = regex.search(received[idx]) # match with incoming output
                    if (not match):
                        fail = True
                        failtext = "line {} received text '{}' does not match expected regex '{}'.".format(idx+1, received[idx], expectedLine)
                        break
                else:
                    #mandatory exact match
                    if(expectedLine != received[idx]):
                        fail = True
                        failtext = "line {} received text '{}' and expected text '{}' does not match".format(idx+1, received[idx], expectedLine)
                        break
                    # end-if
                # end-if
                idx+=1
            # end-for
            if ((len(received) != idx) and (not fail)):
                fail = True
                failtext = "expected number of lines {} (of {}) and received number of lines {} do not match!".format(idx, len(expected), len(received))
            # end-if
            if (fail):
                failedTests.append(f'line: {testline}, name: \'{testname}\'')
                failtext = failtext.replace('\n', '')
                prRed('FAIL: ')
                print(failtext)
            else:
                prGreen('OK!')
            # end-if
            if (line.startswith("#END_CMD")):
                state = "PARSE_CMD"
                continue
            # end-if
            if (line.startswith("#END_TEST")):
                state = "FIND_TEST"
                multiCmd = False
                continue
            # end-if
        # end-if

        if (state == "PARSE_CMD"):
            if(line.startswith("#END_TEST")):
                state = "FIND_TEST"
                multiCmd = False
                continue
            cmd = line.replace("@@CMD@@", gwhisper)
            cmd = cmd.replace('\n', '')
            print(f'Resolve cmd \'{cmd}\'')
            newCmd = cmd.replace("@@PTC@@", certs)
            newCmd = newCmd.replace("@@testResources@@", testResources)
            newCmd = newCmd.replace('\n', '')
            print(f"Execute new command '{newCmd}'")
            # Use bash for space handling
            readProcess = subprocess.Popen(['bash', '-c', newCmd], stdout = subprocess.PIPE, stderr = subprocess.STDOUT) # redirect stderr to stdout
            received = []
            #print (re.split('(?<!") | (?!")', newCmd))
            for outLine in readProcess.stdout:
                received.append(outLine.decode("UTF-8"))
            if multiCmd:
                state = "PARSE_CMD_RESULT"
            else:
                state = "PARSE_RESULT"
            expected = []
            continue
        # end-if

        if (state == "PARSE_RESULT" or state == "PARSE_CMD_RESULT"):
            expected.append(line)
            continue
        # end-if
        if not line:
            break
    # end-while
# end-with

if (state != "FIND_TEST"):
    prRed(f'File ended in the middle of a test case. Did you forget to add #END_TEST somewhere?$') #TODO: Colour and NC
    exit(1)
# end-if

#analyse test result:
print('#################################################################')
print('#################################################################')
numFailed = len(failedTests)
numSucceeded = numTests - numFailed
print(f'{numTests} tests finished. Successful: {numSucceeded}, Failed: {numFailed}')
rc = 0
if(len(failedTests) != 0):
    print('The following tests failed:')
    print(failedTests)
    prRed(f'{numFailed} TESTS FAILED')
    rc = 1
else:
    prGreen(f'ALL {numTests} TESTS SUCCEEDED')
# end-if

# stopping test server
print('Stopping server...')
startProcess.terminate()

# return test result
exit(rc)

