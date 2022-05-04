from ntpath import join
import os
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
print('Blib')
print('Starting server: \'{} --certBasePath {} ...\''.format(testServer, certs))
#startCmd = "{} --certBasePath {}".format(testServer, certs)
#startProcess = subprocess.Popen(startCmd.split())
startProcess = subprocess.Popen([testServer,"--certBase", certs, "&"], stdout=subprocess.PIPE)
print('Blub')
#serverPID = startProcess.pid
time.sleep(0.2)

print('Starting server: \'{}\' ...done'.format(testServer))
print('Running Completion tests...')

# state machine for parser
# Todo: Think about DataStructure for ()
state = "FIND_TEST"
pathToBuild = ""
received = []
expected = []
curline = 0
testline = 0
testname = ""
failedTests = []
numTests = 0

# parse testcase file and execute tests
#file = open(testFile, 'r')
#fileLines = file.readlines()
#while (fileLine in fileLines):
line = " "
with open(testFile) as file:
    while True: # alt: for fileLine in file
        line  = file.readline()
        curline += 1

        if ((line.startswith("#START_TEST")) and (state == "FIND_TEST")):
            testline = curline
            received = []
            testname = line[12:]
            testname = testname.replace("\n", "")
            state = "PARSE_CMD"
            print('#################################################################') 
            print ("Executing test '{}' at line {}".format(testname, testline))
            continue
        # end-if

        if ((line.startswith("#END_TEST")) or (line.startswith("#EXEC_CMD")) and (state == "PARSE_RESULT")):
            numTests += 1
            fail = False
            failtext = ""
            idx = 0

            print(f'EXPECTED: {expected}')

            for expectedLine in expected:
                expectedLine.encode("UTF-8")
                print('--------------------------------------------------------')
                print(f'EXPECTEDLIST: {expected}')
                print(f'RECEIVEDLIST:{received}')
                print(f'EXPECTEDLINE: {expectedLine}')
                print(f'RECEIVEDLINE: {received[idx]}')
                print(f'IDX: {idx}') #Warum sprongt er hier aus dem for raus und geht ins with?


                # TODO: print /n as newline
                if(len(expected) <= idx): # -le = less or equal
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
                    # optional regex --> line starts with "?""
                    expectedLine = expectedLine[1:] # TODO: Muss die line dann an der ersten pos anfangen?
                    regex = re.compile(expectedLine)
                    match = regex.search(received[idx]) # match with incoming output
                    if (not match):
                    #if (not received[idx] == expectedLine): # Wo wird received befüllt?
                        continue
                elif(expectedLine.startswith("/")):
                    # mandatory regex
                    expectedLine = expectedLine[1:] # TODO: Muss die line dann an der ersten pos aufhören? Ist der 1. Teil dann der Regex?
                    print(f'NEW EXPECTED: {expectedLine}')
                    try:
                        regex = re.compile(expectedLine) # regex is not escaped
                    except re.error:
                        print(f'Invalid REGEX at line {curline}. Check your testfile')
                    match = regex.search(received[idx]) # match with incoming output
                    if match:
                        print(f'MATCHED CHARACTERS: {match.group()}')
                    if (not match):
                    #if (not(received[idx] == re.match('\b',expectedLine) )):
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
                # done?
            # end-for
            if ((len(received) != idx) and (not fail)): # and [ ${#received[@]} -ne $idx ]
                fail = True
                # TODO What to do wih #received[@] ?
                failtext = "expected number of lines {} (of {}) and received number of lines {} do not match!".format(idx, len(expected), len(received))
                #break #TODO: Wrong break?
            # end-if
            if (fail):
                failedTests.append(f'line: {testline}, name: \'{testname}\'')
                prRed('FAIL: ')
                print(failtext)
                # TODO echo -e " ${RED}FAIL:${NC} $failtext"
            else:
                prGreen('OK!')
            # end-if
            if (line.startswith("#EXEC_CMD")):
                state = "PARSE_CMD"
                continue
            # end-if
            if (line.startswith("#END_TEST")):
                state = "FIND_TEST"
                continue
            # end-if
        # end-if

        if (state == "PARSE_CMD"):
            cmd = line.replace("@@CMD@@", gwhisper)
            cmd = cmd.replace('\n', '')
            print(f'Resolve cmd \'{cmd}\'')
            newCmd = cmd.replace("@@PTC@@", certs)
            newCmd = newCmd.replace('\n', '')
            print(f"Execute new command '{newCmd}'")
            readProcess = subprocess.Popen(newCmd.split(), stdout = subprocess.PIPE, stderr = subprocess.STDOUT) # redirect stderr to stdout
            # TODO Ist das synchron?
            print("SOMETHING")
            #received = out.stdout
            for outLine in readProcess.stdout:
                # each newline  = new entry in received list
                received.append(outLine.decode("UTF-8"))
            # TODO: Security implications!! What happens, if we pass sth. like rm -f?
            # TODO: Maybe hide error code?
            state = "PARSE_RESULT"
            expected = [] # TODO: expected list komplett leeren?

            continue
        # end-if

        if (state == "PARSE_RESULT"):
            #expected += line
            expected.append(line)
            continue
        # end-if
        if not line:
              # TODO: done < "$testFile"
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
#subprocess.run(['kill', serverPID])
startProcess.terminate()

# return test result
exit(rc)

