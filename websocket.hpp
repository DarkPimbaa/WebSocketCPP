#ifndef WEBSOCKET_HPP
#define WEBSOCKET_HPP

#include <string>
#include <functional>
#include <vector>
#include <unordered_map>
#include <netinet/in.h>
#include <sys/socket.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

class WebSocket {
public:
    struct Frame {
        bool fin;
        bool rsv1;
        bool rsv2;
        bool rsv3;
        uint8_t opcode;
        bool mask;
        uint64_t payload_length;
        uint8_t masking_key[4];
        std::vector<uint8_t> payload;
    };

    enum OpCode {
        CONTINUATION = 0x0,
        TEXT = 0x1,
        BINARY = 0x2,
        CLOSE = 0x8,
        PING = 0x9,
        PONG = 0xA
    };

    WebSocket(int port);
    ~WebSocket();

    void start();
    void stop();
    void onMessage(std::function<void(int client, const std::string&)> callback);
    void sendMessage(int client, const std::string& message);

private:
    int server_fd;
    int port;
    bool running;
    std::function<void(int, const std::string&)> message_callback;
    std::unordered_map<int, bool> clients;

    bool handleHandshake(int client_fd);
    std::string generateAcceptKey(const std::string& client_key);
    Frame parseFrame(const std::vector<uint8_t>& data);
    std::vector<uint8_t> createFrame(const std::string& message, OpCode opcode = TEXT);
    void handleClient(int client_fd);
    std::string base64Encode(const unsigned char* input, int length);
};

#endif