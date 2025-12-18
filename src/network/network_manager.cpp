#include "kermit/network.h"
#include <iostream>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <map>
#include <mutex>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>

namespace kermit {

// NetworkManager implementation
class NetworkManager::Impl {
public:
    bool running_;
    uint16_t listen_port_;
    std::string listen_address_;
    std::atomic<bool> should_stop_;
    
    // Socket management
    int listen_socket_;
    std::map<std::string, int> connections_;
    std::mutex connections_mutex_;
    
    // Thread for network operations
    std::thread network_thread_;
    
    // Callbacks
    ConnectionCallback connection_callback_;
    DataCallback data_callback_;
    
    Impl() : running_(false), listen_port_(0), should_stop_(false), listen_socket_(-1) {}
    
    ~Impl() {
        stop();
    }
    
    bool initialize(uint16_t listen_port, const std::string& listen_address) {
        listen_port_ = listen_port;
        listen_address_ = listen_address;
        
        std::cout << "Network manager initialized on " 
                  << listen_address << ":" << listen_port << std::endl;
        return true;
    }
    
    bool start() {
        if (running_) {
            std::cerr << "Network manager is already running" << std::endl;
            return false;
        }
        
        // Create listen socket
        if (!createListenSocket()) {
            std::cerr << "Failed to create listen socket" << std::endl;
            return false;
        }
        
        // Start network thread
        should_stop_ = false;
        network_thread_ = std::thread(&Impl::networkLoop, this);
        
        running_ = true;
        std::cout << "Network manager started" << std::endl;
        return true;
    }
    
    void stop() {
        if (!running_) return;
        
        running_ = false;
        should_stop_ = true;
        
        // Close listen socket to break accept()
        if (listen_socket_ != -1) {
            close(listen_socket_);
            listen_socket_ = -1;
        }
        
        // Close all connections
        std::lock_guard<std::mutex> lock(connections_mutex_);
        for (auto& conn : connections_) {
            close(conn.second);
        }
        connections_.clear();
        
        // Join network thread
        if (network_thread_.joinable()) {
            network_thread_.join();
        }
        
        std::cout << "Network manager stopped" << std::endl;
    }
    
    bool createListenSocket() {
        listen_socket_ = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_socket_ < 0) {
            std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
            return false;
        }
        
        // Set socket options
        int opt = 1;
        if (setsockopt(listen_socket_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cerr << "Failed to set socket options: " << strerror(errno) << std::endl;
            close(listen_socket_);
            listen_socket_ = -1;
            return false;
        }
        
        // Set non-blocking
        if (fcntl(listen_socket_, F_SETFL, O_NONBLOCK) < 0) {
            std::cerr << "Failed to set non-blocking: " << strerror(errno) << std::endl;
            close(listen_socket_);
            listen_socket_ = -1;
            return false;
        }
        
        // Bind socket
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(listen_port_);
        
        if (listen_address_ == "0.0.0.0") {
            addr.sin_addr.s_addr = INADDR_ANY;
        } else {
            if (inet_pton(AF_INET, listen_address_.c_str(), &addr.sin_addr) != 1) {
                std::cerr << "Invalid listen address: " << listen_address_ << std::endl;
                close(listen_socket_);
                listen_socket_ = -1;
                return false;
            }
        }
        
        if (bind(listen_socket_, (sockaddr*)&addr, sizeof(addr)) < 0) {
            std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
            close(listen_socket_);
            listen_socket_ = -1;
            return false;
        }
        
        // Listen
        if (listen(listen_socket_, SOMAXCONN) < 0) {
            std::cerr << "Failed to listen: " << strerror(errno) << std::endl;
            close(listen_socket_);
            listen_socket_ = -1;
            return false;
        }
        
        std::cout << "Listening on " << listen_address_ << ":" << listen_port_ << std::endl;
        return true;
    }
    
    void networkLoop() {
        std::vector<pollfd> poll_fds;
        
        while (!should_stop_) {
            // Prepare poll file descriptors
            poll_fds.clear();
            
            // Add listen socket
            pollfd listen_pfd{};
            listen_pfd.fd = listen_socket_;
            listen_pfd.events = POLLIN;
            listen_pfd.revents = 0;
            poll_fds.push_back(listen_pfd);
            
            // Add connected sockets
            std::lock_guard<std::mutex> lock(connections_mutex_);
            for (const auto& conn : connections_) {
                pollfd conn_pfd{};
                conn_pfd.fd = conn.second;
                conn_pfd.events = POLLIN | POLLHUP | POLLERR;
                conn_pfd.revents = 0;
                poll_fds.push_back(conn_pfd);
            }
            
            // Wait for events
            int poll_result = poll(poll_fds.data(), poll_fds.size(), 100);
            
            if (poll_result < 0) {
                if (errno == EINTR) continue;
                std::cerr << "Poll error: " << strerror(errno) << std::endl;
                break;
            }
            
            if (poll_result == 0) {
                // Timeout, continue loop
                continue;
            }
            
            // Process events
            for (size_t i = 0; i < poll_fds.size(); ++i) {
                if (poll_fds[i].revents == 0) continue;
                
                if (i == 0 && poll_fds[i].fd == listen_socket_) {
                    // Listen socket event - new connection
                    if (poll_fds[i].revents & POLLIN) {
                        acceptNewConnection();
                    }
                } else {
                    // Connection socket event
                    int sock_fd = poll_fds[i].fd;
                    if (poll_fds[i].revents & (POLLHUP | POLLERR)) {
                        // Connection closed or error
                        handleConnectionClosed(sock_fd);
                    } else if (poll_fds[i].revents & POLLIN) {
                        // Data available
                        handleIncomingData(sock_fd);
                    }
                }
            }
        }
    }
    
    void acceptNewConnection() {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(listen_socket_, (sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                std::cerr << "Accept error: " << strerror(errno) << std::endl;
            }
            return;
        }
        
        // Set non-blocking
        if (fcntl(client_fd, F_SETFL, O_NONBLOCK) < 0) {
            std::cerr << "Failed to set client socket non-blocking: " << strerror(errno) << std::endl;
            close(client_fd);
            return;
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        uint16_t client_port = ntohs(client_addr.sin_port);
        
        std::string connection_id = std::string(client_ip) + ":" + std::to_string(client_port);
        
        std::lock_guard<std::mutex> lock(connections_mutex_);
        connections_[connection_id] = client_fd;
        
        std::cout << "New connection from " << connection_id << std::endl;
        
        // Call connection callback if set
        if (connection_callback_) {
            connection_callback_(connection_id, true);
        }
    }
    
    void handleIncomingData(int sock_fd) {
        char buffer[4096];
        ssize_t bytes_read = recv(sock_fd, buffer, sizeof(buffer), 0);
        
        if (bytes_read < 0) {
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                std::cerr << "Recv error: " << strerror(errno) << std::endl;
                handleConnectionClosed(sock_fd);
            }
            return;
        }
        
        if (bytes_read == 0) {
            // Connection closed
            handleConnectionClosed(sock_fd);
            return;
        }
        
        // Find connection ID
        std::string connection_id;
        {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            for (const auto& conn : connections_) {
                if (conn.second == sock_fd) {
                    connection_id = conn.first;
                    break;
                }
            }
        }
        
        if (!connection_id.empty()) {
            std::vector<uint8_t> data(buffer, buffer + bytes_read);
            
            std::cout << "Received " << bytes_read << " bytes from " << connection_id << std::endl;
            
            // Call data callback if set
            if (data_callback_) {
                data_callback_(connection_id, data);
            }
        }
    }
    
    void handleConnectionClosed(int sock_fd) {
        std::string connection_id;
        
        {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            for (auto it = connections_.begin(); it != connections_.end(); ++it) {
                if (it->second == sock_fd) {
                    connection_id = it->first;
                    close(sock_fd);
                    connections_.erase(it);
                    break;
                }
            }
        }
        
        if (!connection_id.empty()) {
            std::cout << "Connection closed: " << connection_id << std::endl;
            
            // Call connection callback if set
            if (connection_callback_) {
                connection_callback_(connection_id, false);
            }
        }
    }
    
    bool connect(const std::string& host, uint16_t port) {
        std::string connection_id = host + ":" + std::to_string(port);
        
        // Check if already connected
        {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            if (connections_.find(connection_id) != connections_.end()) {
                std::cerr << "Already connected to " << connection_id << std::endl;
                return false;
            }
        }
        
        std::cout << "Connecting to " << connection_id << "..." << std::endl;
        
        // Resolve hostname
        addrinfo hints{};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        
        addrinfo* result;
        int resolve_result = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &result);
        
        if (resolve_result != 0) {
            std::cerr << "Failed to resolve hostname: " << gai_strerror(resolve_result) << std::endl;
            return false;
        }
        
        // Create socket
        int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_fd < 0) {
            std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
            freeaddrinfo(result);
            return false;
        }
        
        // Set non-blocking
        if (fcntl(sock_fd, F_SETFL, O_NONBLOCK) < 0) {
            std::cerr << "Failed to set non-blocking: " << strerror(errno) << std::endl;
            close(sock_fd);
            freeaddrinfo(result);
            return false;
        }
        
        // Connect
        sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(result->ai_addr);
        int connect_result = ::connect(sock_fd, (sockaddr*)addr, sizeof(sockaddr_in));
        
        freeaddrinfo(result);
        
        if (connect_result < 0 && errno != EINPROGRESS) {
            std::cerr << "Failed to connect: " << strerror(errno) << std::endl;
            close(sock_fd);
            return false;
        }
        
        // Add to connections
        {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            connections_[connection_id] = sock_fd;
        }
        
        std::cout << "Connected to " << connection_id << std::endl;
        
        // Call connection callback if set
        if (connection_callback_) {
            connection_callback_(connection_id, true);
        }
        
        return true;
    }
    
    void disconnect(const std::string& connection_id) {
        int sock_fd = -1;
        
        {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            auto it = connections_.find(connection_id);
            if (it != connections_.end()) {
                sock_fd = it->second;
                connections_.erase(it);
            }
        }
        
        if (sock_fd != -1) {
            close(sock_fd);
            
            // Call connection callback if set
            if (connection_callback_) {
                connection_callback_(connection_id, false);
            }
            
            std::cout << "Disconnected from " << connection_id << std::endl;
        }
    }
    
    bool sendData(const std::string& connection_id, const std::vector<uint8_t>& data) {
        int sock_fd = -1;
        
        {
            std::lock_guard<std::mutex> lock(connections_mutex_);
            auto it = connections_.find(connection_id);
            if (it == connections_.end()) {
                std::cerr << "Connection " << connection_id << " not found" << std::endl;
                return false;
            }
            sock_fd = it->second;
        }
        
        if (sock_fd == -1) {
            return false;
        }
        
        ssize_t bytes_sent = send(sock_fd, data.data(), data.size(), 0);
        
        if (bytes_sent < 0) {
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                std::cerr << "Send error: " << strerror(errno) << std::endl;
                return false;
            }
            // Would be blocked, but we'll try again later
            return true;
        }
        
        if (bytes_sent < static_cast<ssize_t>(data.size())) {
            std::cout << "Partial send: " << bytes_sent << "/" << data.size() << " bytes" << std::endl;
            // TODO: Handle partial sends
        }
        
        std::cout << "Sent " << bytes_sent << " bytes to " << connection_id << std::endl;
        return true;
    }
    
    std::vector<uint8_t> receiveData(const std::string& connection_id) {
        // In our implementation, data is handled via callbacks in the network loop
        // This method could be used for synchronous reads if needed
        std::cerr << "receiveData not implemented - use callbacks instead" << std::endl;
        return {};
    }
    
    void setConnectionCallback(ConnectionCallback callback) {
        connection_callback_ = callback;
    }
    
    void setDataCallback(DataCallback callback) {
        data_callback_ = callback;
    }
    
    std::vector<std::string> getActiveConnections() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(connections_mutex_));
        
        std::vector<std::string> active_connections;
        for (const auto& conn : connections_) {
            active_connections.push_back(conn.first);
        }
        
        return active_connections;
    }
    
    bool isConnected(const std::string& connection_id) const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(connections_mutex_));
        return connections_.find(connection_id) != connections_.end();
    }
};

// NetworkManager public interface
NetworkManager::NetworkManager() : impl_(std::make_unique<Impl>()) {}

NetworkManager::~NetworkManager() = default;

bool NetworkManager::initialize(uint16_t listen_port, const std::string& listen_address) {
    return impl_->initialize(listen_port, listen_address);
}

bool NetworkManager::start() {
    return impl_->start();
}

void NetworkManager::stop() {
    impl_->stop();
}

bool NetworkManager::connect(const std::string& host, uint16_t port) {
    return impl_->connect(host, port);
}

void NetworkManager::disconnect(const std::string& connection_id) {
    impl_->disconnect(connection_id);
}

bool NetworkManager::sendData(const std::string& connection_id, const std::vector<uint8_t>& data) {
    return impl_->sendData(connection_id, data);
}

std::vector<uint8_t> NetworkManager::receiveData(const std::string& connection_id) {
    return impl_->receiveData(connection_id);
}

void NetworkManager::setConnectionCallback(ConnectionCallback callback) {
    impl_->setConnectionCallback(callback);
}

void NetworkManager::setDataCallback(DataCallback callback) {
    impl_->setDataCallback(callback);
}

std::vector<std::string> NetworkManager::getActiveConnections() const {
    return impl_->getActiveConnections();
}

bool NetworkManager::isConnected(const std::string& connection_id) const {
    return impl_->isConnected(connection_id);
}

} // namespace kermit