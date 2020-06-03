[![Build Status](https://travis-ci.org/IBM/gWhisper.svg?branch=master)](https://travis-ci.org/IBM/gWhisper)
# gWhisper - A gRPC command line tool
A gRPC command line client.
It allows to invoke gRPC Calls from the commandline, supports tab-completion
and formats the replies in a human readable format.

![example invocation](doc/example.gif)

The main design goals are:

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
- [Manual](doc/Usage.txt)
- [Project scope](doc/PROJECT_SCOPE.md)
- [Contributing](CONTRIBUTING.md)
- [Technical Documentation for Developers](doc/Developer.md)
- [Frequently Asked Questions](doc/FAQ.md)

## Getting started

### Download

Release version: [download](https://github.com/IBM/gWhisper/releases/latest)  
Development version: `git clone https://github.com/IBM/gWhisper.git`

We try to keep the master branch stable. So if you experience bugs / missing features in the latest release, feel free to try the development version.  

### Prerequisites

To be able to build and/or run gWhisper, you need to at least have the following dependencies installed on your system:

- __cmake__
- A C++ compiler
- __gRPC__ [link](https://github.com/grpc/grpc)  
  including the protoc plugin, which is packaged separately in some linux distributions
- __protocolBuffers__ [link](https://github.com/protocolbuffers/protobuf)
- Either __bash__ or __fish__(>=v2.6) or __zsh__ shell

You can install the prerequisites with:

Fedora:

    dnf install cmake gcc protobuf protobuf-devel grpc grpc-devel grpc-plugins

Arch Linux:
  
    pacman -S cmake gcc protobuf protobuf-c grpc

On some (e.g. ubuntu 16.04) distributions we tried, gRPC and/or protobuf packages seem to be not available, outdated or incomplete (missing gRPC protoc plugin).
In this case, please build and install gRPC and protocolBuffers from the official sources.

### Build

Build the code

    ./build.sh

Executables are now available in the `build` folder.


If you do not want to [install](#Install) gWhisper, you need to source the completion file (for tab completion):  
`source ./complete.bash` or `source ./complete.fish`

### Install

You may use the cmake-provided `install` target:

    cd build
    make install

Alternatively just copy the following files to the appropriate locations:

    cp build/gwhisper /usr/local/bin/
    cp complete.bash /usr/share/bash-completion/completions/gwhisper
    cp complete.fish /usr/share/fish/vendor_completions.d/gwhisper.fish

If you are using zsh, add following lines into your `~/.zshrc` for tab-completion support based on the bash completion script.

    autoload bashcompinit && bashcompinit
    source /usr/share/bash-completion/completions/gwhisper

## Examples and Test-Server
Every element except the hostname in the following example CLI invocations can be tab-completed in the bash, fish or zsh shell.


Simple example of an unary RPC with only one field in the request message:

    gwhisper exampledomain.org bakery orderCookies amount=5

Nested arguments (the config field contains a sub-message):

    gwhisper exampledomain.org bakery orderCookies config=:chocolate=true smarties=false: amount=5

IPv6 address and explicit TCP port with an enum typed field:

    gwhisper [2001:db8::2:1]:50059 bakery orderCookies type=ChunkyStyle amount=0x7

Feel free to use the _Test-Server_ shipped with gWhisper and try out gWhisper on your own.
It is located in `build/testServer` and implements example RPCs which cover almost the
complete gRPC and protocol buffers function set.

## Escaping and special characters
gWhisper control characters (` :,`) in string fields need to be escaped:

    gwhisper 127.0.0.1 examples.ScalarTypeRpcs capitalizeString text=special: characters: :,::
    2020-03-24 12:12:04: Received message:
    | text = "SPECIAL CHARACTERS ,:"
    RPC succeeded :D

Shell control characters need to be escaped via shell mechanisms (works as expected):

    gwhisper 127.0.0.1 examples.ScalarTypeRpcs capitalizeString text=shell: Characters::: \\\'\"\$
    2020-03-24 12:14:51: Received message:
    | text = "SHELL CHARACTERS: \'"$"
    RPC succeeded :D
    

## Current development status

Basic functionality is implemented, but you may experience bugs.
Feel free to try it out and provide feedback/contributions.

What is working:

- Tab Completion (bash, fish and zsh only)
- Calling RPCs (unary + streaming)
- Input and output of all protocol buffer types

Some notable things which are not yet working:

- Security: Authentication / Encryption of channels __(Planned in v0.4)__
- Using Proto files instead of Reflection API (currently gWhisper only works with servers which have reflection enabled) __(Planned in v0.4)__
- Recursive datastructures  __(Planned in v0.4)__
- Completion: Support for shells other than bash/fish
- Performance: Caching of reflection queries

## Supported platforms

Development and testing is done on Fedora Linux 31, Arch Linux and Ubuntu.
We expect no bigger problems with building and running this software on different linux distributions.

## Reporting issues

We really appreciate any kind of feedback :-) So don't hesitate to open a new issue.  
Please use the GitGub [issues tab](https://github.com/ibm/gWhisper/issues).
Be sure to search issues first to avoid duplicate entries.

## Contribute

Please have a look at [CONTRIBUTE.md](CONTRIBUTING.md) for general information about contributing. 
Some more technical documentation can be found here: [Technical Documentation for Developers](doc/Developer.md).
