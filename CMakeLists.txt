cmake_minimum_required(VERSION 3.10)
project(WebSocketCPP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configurar linkagem estática
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(CMAKE_BUILD_TYPE Release)

# Encontrar OpenSSL estático
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL REQUIRED)

add_executable(websocket_server
    main.cpp
    websocket.cpp
)

# Linkar bibliotecas estaticamente
target_link_libraries(websocket_server
    PRIVATE
    -static
    OpenSSL::SSL
    OpenSSL::Crypto
    -static-libgcc
    -static-libstdc++
    pthread
)
