#include "websocket.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <algorithm>

WebSocket::WebSocket(int port) : port(port), running(false) {}

WebSocket::~WebSocket() {
    stop();
}

void WebSocket::start() {
    struct sockaddr_in address;
    int opt = 1;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        throw std::runtime_error("Socket creation failed");
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        throw std::runtime_error("setsockopt failed");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        throw std::runtime_error("Bind failed");
    }

    if (listen(server_fd, 3) < 0) {
        throw std::runtime_error("Listen failed");
    }

    running = true;
    std::cout << "WebSocket server started on port " << port << std::endl;

    while (running) {
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
        
        if (client_fd < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        std::thread client_thread(&WebSocket::handleClient, this, client_fd);
        client_thread.detach();
    }
}

void WebSocket::stop() {
    running = false;
    close(server_fd);
}

void WebSocket::onMessage(std::function<void(int, const std::string&)> callback) {
    message_callback = callback;
}

std::string WebSocket::generateAcceptKey(const std::string& client_key) {
    std::string magic_string = client_key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(magic_string.c_str()), magic_string.length(), hash);
    return base64Encode(hash, SHA_DIGEST_LENGTH);
}

std::string WebSocket::base64Encode(const unsigned char* input, int length) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, input, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);

    std::string result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);

    return result;
}

bool WebSocket::handleHandshake(int client_fd) {
    char buffer[1024] = {0};
    read(client_fd, buffer, 1024);

    std::string data(buffer);
    std::string key;
    
    size_t key_pos = data.find("Sec-WebSocket-Key: ");
    if (key_pos != std::string::npos) {
        key = data.substr(key_pos + 19, 24);
    }

    std::string accept_key = generateAcceptKey(key);
    
    std::string response = "HTTP/1.1 101 Switching Protocols\r\n"
                          "Upgrade: websocket\r\n"
                          "Connection: Upgrade\r\n"
                          "Sec-WebSocket-Accept: " + accept_key + "\r\n\r\n";

    send(client_fd, response.c_str(), response.length(), 0);
    return true;
}

WebSocket::Frame WebSocket::parseFrame(const std::vector<uint8_t>& data) {
    Frame frame;
    size_t pos = 0;

    frame.fin = (data[pos] & 0x80) != 0;
    frame.rsv1 = (data[pos] & 0x40) != 0;
    frame.rsv2 = (data[pos] & 0x20) != 0;
    frame.rsv3 = (data[pos] & 0x10) != 0;
    frame.opcode = data[pos] & 0x0F;
    pos++;

    frame.mask = (data[pos] & 0x80) != 0;
    frame.payload_length = data[pos] & 0x7F;
    pos++;

    if (frame.payload_length == 126) {
        frame.payload_length = (data[pos] << 8) | data[pos + 1];
        pos += 2;
    } else if (frame.payload_length == 127) {
        frame.payload_length = 0;
        for (int i = 0; i < 8; i++) {
            frame.payload_length = (frame.payload_length << 8) | data[pos + i];
        }
        pos += 8;
    }

    if (frame.mask) {
        memcpy(frame.masking_key, &data[pos], 4);
        pos += 4;
    }

    frame.payload.resize(frame.payload_length);
    memcpy(frame.payload.data(), &data[pos], frame.payload_length);

    if (frame.mask) {
        for (size_t i = 0; i < frame.payload_length; i++) {
            frame.payload[i] ^= frame.masking_key[i % 4];
        }
    }

    return frame;
}

std::vector<uint8_t> WebSocket::createFrame(const std::string& message, OpCode opcode) {
    std::vector<uint8_t> frame;
    size_t message_length = message.length();

    // First byte: FIN + RSV + Opcode
    frame.push_back(0x80 | opcode); // FIN=1, RSV1-3=0

    // Second byte: Mask + Payload length
    if (message_length <= 125) {
        frame.push_back(message_length);
    } else if (message_length <= 65535) {
        frame.push_back(126);
        frame.push_back((message_length >> 8) & 0xFF);
        frame.push_back(message_length & 0xFF);
    } else {
        frame.push_back(127);
        for (int i = 7; i >= 0; i--) {
            frame.push_back((message_length >> (i * 8)) & 0xFF);
        }
    }

    // Add payload
    frame.insert(frame.end(), message.begin(), message.end());
    return frame;
}

void WebSocket::handleClient(int client_fd) {
    clients[client_fd] = true;

    if (!handleHandshake(client_fd)) {
        close(client_fd);
        clients.erase(client_fd);
        return;
    }

    std::vector<uint8_t> buffer(1024);
    while (running && clients[client_fd]) {
        ssize_t bytes_read = recv(client_fd, buffer.data(), buffer.size(), 0);
        
        if (bytes_read <= 0) {
            break;
        }

        buffer.resize(bytes_read);
        Frame frame = parseFrame(buffer);

        if (frame.opcode == OpCode::TEXT) {
            std::string message(frame.payload.begin(), frame.payload.end());
            if (message_callback) {
                message_callback(client_fd, message);
            }
        } else if (frame.opcode == OpCode::CLOSE) {
            break;
        }
    }

    close(client_fd);
    clients.erase(client_fd);
}

void WebSocket::sendMessage(int client_fd, const std::string& message) {
    if (clients.find(client_fd) == clients.end()) {
        return;
    }

    auto frame = createFrame(message, OpCode::TEXT);
    send(client_fd, frame.data(), frame.size(), 0);
}