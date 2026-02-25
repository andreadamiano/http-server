# Embedded HTTP Server in C

## Description

This project is a recreation of an embedded HTTP server implemented in C, designed to run on embedded Linux devices. The server leverages TCP connections using sockets for reliable communication and employs multiplexing with epoll for efficient handling of multiple connections. It supports binding multiple sockets to the same port, enabling concurrent request processing.

## Features

- **TCP Socket Communication**: Utilizes standard socket programming for TCP-based HTTP communication.
- **Epoll Multiplexing**: Implements epoll for non-blocking I/O operations, allowing the server to handle multiple client connections efficiently.
- **Multi-Socket Binding**: Binds multiple sockets to the same port to support high concurrency.
- **Embedded Linux Compatibility**: Optimized for resource-constrained embedded environments.

## Building the Project

To build the project, ensure you have GCC installed and run the following commands:

```bash
make clean
make
```

For a debug build with additional flags:

```bash
make debug
```

## Running the Server

After building, run the server with:

```bash
make run
```

Or directly:

```bash
./build/main
```

## Project Structure

- `src/`: Source code directory containing C files.
- `build/`: Build directory for object files and the executable.
- `Makefile`: Build configuration and rules.

## Dependencies

- GCC compiler
- Standard C libraries (no external dependencies required)

