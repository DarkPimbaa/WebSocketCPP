# WebSocket C++ Library

A lightweight WebSocket server implementation in C++ that enables real-time communication between web browsers and server applications. This project implements the WebSocket protocol (RFC 6455) from scratch, with minimal external dependencies.

## Features

- Pure C++ implementation of WebSocket protocol
- Support for text messages
- Multi-threaded client handling
- Proper WebSocket handshake implementation
- Available in both dynamic and static linking
- Simple and clean API
- Browser-based test client included

## Requirements

- C++17 compiler
- CMake 3.10 or higher
- OpenSSL development libraries

## Building

```bash
mkdir build
cd build
cmake ..
make
```

For static linking, the CMake configuration is already set up to create a standalone executable.

## Usage

1. Start the WebSocket server:
```bash
./websocket_server
```

2. Start a local HTTP server to serve the test page:
```bash
python3 -m http.server 8000
```

3. Open your web browser and navigate to:
```
http://localhost:8000/index.html
```

## Project Structure

- `websocket.hpp` - WebSocket class definition
- `websocket.cpp` - WebSocket implementation
- `main.cpp` - Example usage
- `index.html` - Test client interface

## License

This project is open source and available under the MIT License.
