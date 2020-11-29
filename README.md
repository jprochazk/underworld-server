# UnderWorld Server

This repository hosts the game server for an online browser-based platformer.

### Prerequisites

To build the server, you need:
* A C++17 compiler (clang, gcc)
* [Conan](https://conan.io/downloads.html)
* [XMake](https://xmake.io/#/getting_started?id=installation)

Run the following commands to verify that you've installed the prerequisites correctly:
```
$ conan --version
$ xmake --version
```

### Build

Both Windows and Linux builds are supported. 

To configure and build, execute the following:
```
$ xmake
```

Before running, optionally initialize the config:
```
$ cp config.example.json config.json
```
And fill in any desired values.

Finally, the server can be run by executing:
```
$ xmake run server
```
Which will start the server on the address/port specified in the config, or the default `127.0.0.1:8080`.

### Tests

Tests are written using [GoogleTest](https://github.com/google/googletest).

```
$ xmake run tests
```
