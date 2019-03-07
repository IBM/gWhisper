# gWhisper - A gRPC command line tool
A gRPC command line client.
It allows to invoke gRPC Calls from the commandline and formats the replies
in a human readable format.

![example invocation](example.gif)

The main design goals are:

- Reflection support (no proto files required)
- Tab completion for
    - services
    - methods
    - method arguments (currently only supported in Bash)
      including nested types
- Usable directly in the shell
- Designed with usability in mind

Have a look at the [project scope](PROJECT_SCOPE.md) for details.

Synopsis:

    gwhisper [OPTIONS] <hostname>[:port] <service> <method> [<fieldName>=FIELD_VALUE ]...

Execute `gwhisper --help` or click [here](doc/Usage.txt) to get detailed information and examples on how to use the tool.

## Download gWhisper

### Release version
Download the latest release [here](https://github.com/IBM/gWhisper/releases/latest)

### Development version
Clone the repository

    git clone https://github.com/IBM/gWhisper.git

To be able to build and run tests, initialize third-party submodules (this will download _googletest_)

    cd gWhisper
    git submodule update --init

NOTE: If you retrieved gWhisper from GitHub as an archive (zip or tar.gz), submodules are not included, as GitHub currently does not fully support submodules.

## Prerequisites

To be able to build and/or run gWhisper, you need to at least have the following dependencies installed on your system:

- cmake
- A C++ compiler
- gRPC [link](https://github.com/grpc/grpc)
    including the protoc plugin, which is packaged separately in some linux distributions
- protocolBuffers [link](https://github.com/protocolbuffers/protobuf)

On Fedora you can install the prerequisites with:

    yum install cmake gcc-c++ protoc grpc grpc-devel grpc-plugins

On other distributions we tried, gRPC and/or protobuf packages seem to be not available, outdated or incomplete (missing gRPC protoc plugin).
In this case, please build and install gRPC and protocolBuffers from the official sources.

## Build and run

Build the code

    ./build.sh

Source the bash completion file (for tab completion)

    . ./complete.bash

Run the executable (use TabCompletion):

    ./build/gwhisper [<Options>] <IP-Address> <Service> <Method> <Args>

NOTE:
You may set the environment variable `GWHISPER_BUILD_VERSION` to a string of your choice before building.
This string will end up as part of the version string, returned when calling `gWhisper --version`.

## Current development status

This is in a pre-release state. Basic functionality is implemented, but you may experience bugs.
Feel free to try it out and provide feedback/contributions.

What is working:

- Tab Completion (bash only)
- Calling RPCs (unary + server-streaming)
- Output of all types supported by protocol buffers

Some notable things which are not yet working:

- Input: OneOf fields
- Input: Client streaming RPCs
- Input: Escaping of control characters (":@.(,")
- Completion: Support for shells other than BASH (e.g. zsh, fish)
- Security: Authentication / Encryption of channels
- Performance: Caching of reflection queries

## Supported platforms

All development and testing is done on ubuntu linux (mostly 16.04). We expect no bigger problems
with building and running this software on different linux distributions.
    
## Reporting issues

Please use the GitGub [issues tab](https://github.com/ibm/gWhisper/issues).
Be sure to search issues first to avoid duplicate entries.

## Contribute

Please have a look at [CONTRIBUTE.md](CONTRIBUTING.md).
