#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>

// Simple TCP server for testing
void runSimpleServer(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Server: Failed to create socket" << std::endl;
        return;
    }
    
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Server: Failed to bind" << std::endl;
        close(server_fd);
        return;
    }
    
    if (listen(server_fd, 5) < 0) {
        std::cerr << "Server: Failed to listen" << std::endl;
        close(server_fd);
        return;
    }
    
    std::cout << "Server: Listening on port " << port << std::endl;
    
    // Accept one connection
    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
    
    if (client_fd < 0) {
        std::cerr << "Server: Failed to accept connection" << std::endl;
        close(server_fd);
        return;
    }
    
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    std::cout << "Server: Accepted connection from " << client_ip << std::endl;
    
    // Read data
    char buffer[1024];
    ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer));
    
    if (bytes_read > 0) {
        std::string message(buffer, bytes_read);
        std::cout << "Server: Received " << bytes_read << " bytes: " << message << std::endl;
        
        // Send response
        std::string response = "ACK: " + message;
        write(client_fd, response.c_str(), response.size());
        std::cout << "Server: Sent response" << std::endl;
    }
    
    close(client_fd);
    close(server_fd);
    std::cout << "Server: Connection closed" << std::endl;
}

// Simple TCP client for testing
bool testSimpleClient(const std::string& host, int port, const std::string& message) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        std::cerr << "Client: Failed to create socket" << std::endl;
        return false;
    }
    
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
        std::cerr << "Client: Invalid address" << std::endl;
        close(sock_fd);
        return false;
    }
    
    if (connect(sock_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Client: Failed to connect" << std::endl;
        close(sock_fd);
        return false;
    }
    
    std::cout << "Client: Connected to " << host << ":" << port << std::endl;
    
    // Send message
    if (write(sock_fd, message.c_str(), message.size()) < 0) {
        std::cerr << "Client: Failed to send data" << std::endl;
        close(sock_fd);
        return false;
    }
    
    std::cout << "Client: Sent " << message.size() << " bytes" << std::endl;
    
    // Receive response
    char buffer[1024];
    ssize_t bytes_read = read(sock_fd, buffer, sizeof(buffer));
    
    if (bytes_read > 0) {
        std::string response(buffer, bytes_read);
        std::cout << "Client: Received " << bytes_read << " bytes: " << response << std::endl;
    }
    
    close(sock_fd);
    std::cout << "Client: Connection closed" << std::endl;
    return true;
}

int main() {
    std::cout << "Testing Kermit network functionality with simple TCP test..." << std::endl;
    
    // Start server in background
    std::thread server_thread(runSimpleServer, 9052);
    
    // Give server time to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Test client connection
    std::string test_message = "Hello Kermit!";
    bool success = testSimpleClient("127.0.0.1", 9052, test_message);
    
    // Wait for server to finish
    server_thread.join();
    
    if (success) {
        std::cout << "Network test: PASSED" << std::endl;
    } else {
        std::cout << "Network test: FAILED" << std::endl;
        return 1;
    }
    
    return 0;
}