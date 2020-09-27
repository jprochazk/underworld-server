# UnderWorld Server

This repository hosts the game server for an online browser-based platformer.

### Prerequisites

To build the server, you need:
1. A C++17 compiler (clang, gcc)
2. [Premake 5.0+](https://premake.github.io/download.html#v5)
3. [Conan 1.26.0+](https://conan.io/downloads.html)

Run the following commands to verify that you've installed the prerequisites correctly:
```
$ premake5 --version
$ conan --version
```

To see the list of available commands and options, execute:
```
$ premake5 help
```

### Build

Currently, only Linux is supported. Windows/MacOS support is not planned.

To build, execute the following:
```
$ premake5 install
$ premake5 gmake2
$ make server
```

Before running, optionally initialize the config:
```
$ cp config.example.json config.json
```
And fill in values as needed.

Finally, the server can be run as:
```
$ ./server
```
Which will start the server on the requested address/port, with the requested number of threads.

### Tests

Tests are written using [GoogleTest](https://github.com/google/googletest).

```
$ make tests
$ ./tests
```
