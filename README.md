[![Build Status](https://travis-ci.org/IBM/gWhisper.svg?branch=master)](https://travis-ci.org/IBM/gWhisper)
# gWhisper - A gRPC command line tool
A gRPC command line client.
It allows to invoke gRPC Calls from the commandline, supports tab-completion
and formats the replies in a human readable format.

![example invocation](doc/exampleHello.gif)

The main features are:

- Reflection support (no proto files required)
- Tab completion (currently supported in fish and bash) for
    - services
    - methods
    - method arguments, including nested types
- Usable directly in the shell

Have a look at the [project scope](doc/PROJECT_SCOPE.md) for details.

Synopsis:

    gwhisper [OPTIONS] <hostname>[:port] <service> <method> [<fieldName>=FIELD_VALUE ]...

Execute `gwhisper --help` or click [here](doc/Usage.txt) to get detailed information and examples on how to use the tool.

Quick links:
- [Usage Manual](doc/Usage.txt)
- [Project scope](doc/PROJECT_SCOPE.md)
- [Contributing](CONTRIBUTING.md)
- [Technical Documentation for Developers](doc/Developer.md)
- [Frequently Asked Questions](doc/FAQ.md)

## Getting started

### Download

Release version: [download](https://github.com/IBM/gWhisper/releases/latest)  
Development version: `git clone https://github.com/IBM/gWhisper.git`

### Prerequisites

To be able to build and/or run gWhisper, you need to have the following dependencies installed on your system:

Mandatory:

- __cmake__
- A C++ compiler, e.g. __gcc__
- Either __bash__, __fish__(>=v2.6) or __zsh__ shell

Optional:

- __openssl__ Required if building the test server to generate test certificates. To disable building the test server have a look at [Advanced Building](#advanced-building)
- __gRPC__ If you have gRPC installed on your system this will be used to build against,
otherwise the gWhisper build system will download, build and statically link
gRPC automatically.

### Build

To simply build gWhisper in default configuration (including the test server),
just execute

    ./build.sh

Executables are now available in the `build/bin` folder.


If you do not want to [install](#Install) gWhisper, you need to source the
completion file (for tab completion):  

    source ./complete.bash` or `source ./complete.fish

### Install

You may use the cmake-provided `install` target:

    cd build
    make install

Alternatively just copy the following files to the appropriate locations:

    cp build/bin/gwhisper /usr/local/bin/
    cp complete.bash /usr/share/bash-completion/completions/gwhisper
    cp complete.fish /usr/share/fish/vendor_completions.d/gwhisper.fish

If you are using zsh, add the following lines to your `~/.zshrc` for tab-completion support based on the bash completion script.

    autoload bashcompinit && bashcompinit
    source /usr/share/bash-completion/completions/gwhisper

### Advanced building

If you plan to build gWhisper for packaging or as a developer / contributor, the
following information might be useful:

#### Directly use CMake to build

Follow the usual CMake flow:

    mkdir build
    cd build
    cmake .. <cmake-options>
    make -j<numCores>

#### CMake options
You can list gWhisper related build options via

    cmake -LAH .. | grep --context 1 GWHISPER

The following are the most relevant options:

- `GWHISPER_BUILD_TESTS` (default = OFF): Build unit and functional tests
- `GWHISPER_BUILD_TESTSERVER` (default = ON): Build the testserver. This requires openssl to be installed for certificate creation.
- `GWHISPER_FORCE_BUILDING_GRPC` (default = OFF): Do not use a system installation of gRPC even if found. Instead always download and build gRPC from source

You can set options in CMake with the `-D` flag. For example:

    cmake .. -D GWHISPER_BUILD_TESTSERVER=OFF

## Examples and Test-Server
Every element except the hostname in the following example CLI invocations can be tab-completed in the bash, fish or zsh shell.


Simple example of an unary RPC with only one field in the request message:

    gwhisper exampledomain.org bakery orderCookies amount=5

Nested arguments (the config field contains a sub-message):

    gwhisper exampledomain.org bakery orderCookies config=:chocolate=true smarties=false: amount=5

IPv6 address and explicit TCP port with an enum typed field:

    gwhisper [2001:db8::2:1]:50059 bakery orderCookies type=ChunkyStyle amount=0x7

Feel free to use the _Test-Server_ shipped with gWhisper and try out gWhisper on your own.
It is located in `build/bin/testServer` and implements example RPCs which cover almost the
complete gRPC and protocol buffers function set.

## Current development status

Basic functionality is implemented, but you may experience bugs.
Feel free to try it out and provide feedback/contributions.

What is working:

- Tab Completion (bash, fish and zsh only)
- Calling RPCs (unary + streaming)
- Input and output of all protocol buffer types

Some notable things which are not yet working:

- Security: Authentication / Encryption of channels __(Planned in >=v4.0)__
- Using Proto files instead of Reflection API (currently gWhisper only works with servers which have reflection enabled)
- Performance: Caching of reflection queries

## Supported platforms

Development and testing is done on Fedora Linux 33 and Arch Linux.
We expect no bigger problems with building and running this software on different linux distributions.

## Reporting issues

We really appreciate any kind of feedback :-) So don't hesitate to open a new issue.  
Please use the GitGub [issues tab](https://github.com/ibm/gWhisper/issues).
Be sure to search issues first to avoid duplicate entries.

## Contribute

Please have a look at [CONTRIBUTE.md](CONTRIBUTING.md) for general information about contributing. 
Some more technical documentation can be found here: [Technical Documentation for Developers](doc/Developer.md).
