#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "src/include/kermit/network.h"

void testServer() {
    std::cout << "Starting network server test..." << std::endl;
    
    kermit::NetworkManager network_manager;
    
    // Set up callbacks
    network_manager.setConnectionCallback([](const std::string& conn_id, bool connected) {
        if (connected) {
            std::cout << "Server: Connection established with " << conn_id << std::endl;
        } else {
            std::cout << "Server: Connection closed with " << conn_id << std::endl;
        }
    });
    
    network_manager.setDataCallback([](const std::string& conn_id, const std::vector<uint8_t>& data) {
        std::cout << "Server: Received " << data.size() << " bytes from " << conn_id << std::endl;
        std::string message(data.begin(), data.end());
        std::cout << "Server: Message: " << message << std::endl;
        
        // Echo back
        std::vector<uint8_t> response(data.begin(), data.end());
        // Would send here if we had access to network manager
    });
    
    if (!network_manager.initialize(9051, "127.0.0.1")) {
        std::cerr << "Failed to initialize network manager" << std::endl;
        return;
    }
    
    if (!network_manager.start()) {
        std::cerr << "Failed to start network manager" << std::endl;
        return;
    }
    
    std::cout << "Server: Network manager started on port 9051" << std::endl;
    
    // Let it run for a while
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    network_manager.stop();
    std::cout << "Server: Network manager stopped" << std::endl;
}

void testClient() {
    std::cout << "Starting network client test..." << std::endl;
    
    // Give server time to start
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    kermit::NetworkManager network_manager;
    
    // Set up callbacks
    network_manager.setConnectionCallback([](const std::string& conn_id, bool connected) {
        if (connected) {
            std::cout << "Client: Connected to " << conn_id << std::endl;
        } else {
            std::cout << "Client: Disconnected from " << conn_id << std::endl;
        }
    });
    
    network_manager.setDataCallback([](const std::string& conn_id, const std::vector<uint8_t>& data) {
        std::cout << "Client: Received " << data.size() << " bytes from " << conn_id << std::endl;
        std::string message(data.begin(), data.end());
        std::cout << "Client: Message: " << message << std::endl;
    });
    
    if (!network_manager.initialize(0, "127.0.0.1")) {
        std::cerr << "Failed to initialize network manager" << std::endl;
        return;
    }
    
    if (!network_manager.start()) {
        std::cerr << "Failed to start network manager" << std::endl;
        return;
    }
    
    std::cout << "Client: Network manager started" << std::endl;
    
    // Connect to server
    if (!network_manager.connect("127.0.0.1", 9051)) {
        std::cerr << "Failed to connect to server" << std::endl;
        return;
    }
    
    // Give connection time to establish
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Send test message
    std::string test_message = "Hello from Kermit network test!";
    std::vector<uint8_t> message_data(test_message.begin(), test_message.end());
    
    if (!network_manager.sendData("127.0.0.1:9051", message_data)) {
        std::cerr << "Failed to send data" << std::endl;
    } else {
        std::cout << "Client: Sent test message to server" << std::endl;
    }
    
    // Let it run for a while
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    network_manager.stop();
    std::cout << "Client: Network manager stopped" << std::endl;
}

int main() {
    std::cout << "Testing Kermit network functionality..." << std::endl;
    
    // Start server in background
    std::thread server_thread(testServer);
    
    // Give server time to start
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Run client in main thread
    testClient();
    
    // Wait for server to finish
    server_thread.join();
    
    std::cout << "Network test completed!" << std::endl;
    return 0;
}