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

### Including in Your Project

```cpp
#include <websocket.hpp>
```

### CMake Integration

Add to your CMakeLists.txt:

```cmake
find_package(OpenSSL REQUIRED)
find_library(WEBSOCKET_LIB websocket)

add_executable(your_app main.cpp)
target_link_libraries(your_app 
    PRIVATE 
    ${WEBSOCKET_LIB}
    OpenSSL::SSL 
    OpenSSL::Crypto 
    pthread
)
```

### Basic Example

```cpp
#include <websocket.hpp>

int main() {
    // Server example
    WebSocket server(8080);
    server.onMessage([](const std::string& msg) {
        std::cout << "Received: " << msg << std::endl;
    });
    server.start();

    // Client example
    WebSocket client("ws://localhost:8080");
    client.connect();
    client.send("Hello, WebSocket!");

    return 0;
}
```

## Project Structure

- `websocket.hpp` - WebSocket class definition
- `websocket.cpp` - WebSocket implementation
- `main.cpp` - Example usage
- `index.html` - Test client interface

## License

This project is open source and available under the MIT License.
