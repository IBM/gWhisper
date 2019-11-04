# Scope and design goals of the gWhisper CLI project
This document defines the scope of gWhisper. It should be helpful to
decide if a feature request or pull request will be accepted or not.

However if you have ideas which do not quite fit the scope and you are willing to implement
them, feel free to start a discussion.

## User personas
The scope of the project is probably defined best by looking at the intended users:

### A gRPC service developer
- Maybe uses gRPC for the first time
- Wants to get to know gRPC by exploring a test server (hello world)
- Wants to test and debug a gRPC service implementation

### A gRPC API user
- Maybe uses gRPC for the first time
- Has to interface a badly documented service implementation
- Wants to explore a gRPC service, as well as its exported RPCs and message types
- Wants to execute RPCs to learn how the server reacts

### A person giving technical support
- Has deep knowledge of the service implementations
- Wants to execute RPCs to recover a corrupted server state
- Wants to execute RPCs to trigger actions which are not possible in static client implementations

## What is important:
- Tab completion
    - Big plus in usability
    - No need to look at documentation
- Reflection support
    - Discoverability: all provided services of the server are offered to the CLI user 
    - User does not have to care about proto files at all. In fact he or she does
      not even need to know what a proto file is :-)
- Colors
    - Modern terminals support color
    - Complex data structures are much better to read with color highlighting
- Documentation
    - Users should not need to use internet search engines or write mails/issues to learn how to use the tool
- The tool should be usable in a terminal
    - All intended users are developers and assumed to be familiar with CLI tools
    - A CLI tool tends to be more versatile and time-less than graphical interfaces
    - Allows flexible use of the tool (e.g. via SSH, in scripts, etc.)

## Nice to have, but not necessary:
- Custom output formatting
    - This tends to let gRPC be used as a production tool, which is not the primary goal of gWhisper
- Syntax compatibility between releases
    - This would allow use of the tool in long-living scripts or for _Machine to Machine_ communication production environments, which is not a goal of gWhisper.

## What is not important:
- Performance for executing gRPC calls
    The tool is not intended to be used for executing large numbers of RPCs or
    for transfering big chunks of data.
- Output formatting in machine readable form (think JSON, XML, etc)
    The tool is intended as a User Interface not for use in production environments.
    If it is desired to process replies of a server with an algorithm, users
    should directly implement their own gRPC client. (gRPC provides excellent
    scripting interfaces)
