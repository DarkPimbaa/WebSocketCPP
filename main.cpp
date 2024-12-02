#include "websocket.hpp"
#include <iostream>

int main() {
    try {
        WebSocket server(8080);
        
        // Callback para receber mensagens
        server.onMessage([&server](int client, const std::string& message) {
            std::cout << "Received message from client " << client << ": " << message << std::endl;
            
            // Respondendo ao cliente
            server.sendMessage(client, "Server recebeu: " + message);
        });

        // Inicia o servidor
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}