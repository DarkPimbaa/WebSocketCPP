cmake_minimum_required(VERSION 3.10)
project(WebSocketCPP VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Set include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Library source files
set(LIB_SOURCES
    websocket.cpp
)

# Create static library
add_library(websocket_static STATIC ${LIB_SOURCES})
set_target_properties(websocket_static PROPERTIES OUTPUT_NAME websocket)
target_link_libraries(websocket_static PRIVATE OpenSSL::SSL OpenSSL::Crypto pthread)

# Create shared library
add_library(websocket_shared SHARED ${LIB_SOURCES})
set_target_properties(websocket_shared PROPERTIES OUTPUT_NAME websocket)
target_link_libraries(websocket_shared PRIVATE OpenSSL::SSL OpenSSL::Crypto pthread)

# Example executable
add_executable(websocket_example main.cpp)
target_link_libraries(websocket_example PRIVATE websocket_static)

# Installation rules
install(TARGETS websocket_static websocket_shared
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)
install(FILES include/websocket.hpp DESTINATION include)

# Set output directories
set_target_properties(websocket_static websocket_shared
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
