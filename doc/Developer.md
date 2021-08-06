# Technical Documentation for Developers
This file includes documentation about stuff, usually not required by the end-user, but which might be interesting if you plan to debug or contribute to gWhisper

## Build environment and packaging
### Version string
During build an attempt is made to extract version information from GIT.
However in some situations this will not work (i.e. Build in downloaded zip file).

You may set the environment variable `GWHISPER_BUILD_VERSION` to a string of your choice before building.
This string will end up as part of the version string, returned when calling `gWhisper --version`.


## Test environment
gWhisper uses both function and unit testing. 

To be able to build and run tests, enable them in CMake

    cmake .. -D GWHISPER_BUILD_TESTS=ON

To execute all tests run

    cd build
    ctest --verbose


### Function tests
For function testing we wrote a simple bash test framework which runs gWhisper against the testServer and compares the received output with te expected output. 
The test framework is implemented in `tests/functionTests/runFunctionTest.sh`. 
The script receives several arguments to build paths. Best you look at the output
of `ctest --verbose` to find out how to execute a single test in isolation.

The test description file contains multiple tests like this example:

    #START_TEST requestStream0Count
    @@CMD@@ 127.0.0.1 examples.StreamingRpcs requestStreamCountMessages
    /.* Received message:
    | number = 0 (0x00000000)
    RPC succeeded :D
    #END_TEST

    #START_TEST requestStream2Count
    @@CMD@@ 127.0.0.1 examples.StreamingRpcs requestStreamCountMessages :: ::
    /.* Received message:
    | number = 2 (0x00000002)
    RPC succeeded :D
    #END_TEST

Each test is to be enclosed by `#START_TEST <TEST NAME>` and `#END_TEST`. 
The first line of a test represents the command to be executed. `@@CMD@@` is replaced with the gwhisper executable during test execution. 
Subsequent lines denote the expected output. It a line starts with `/` it is interpretet as a regex.

### Unit tests
We use google test. Test implementations can be found in

    tests/unitTests
