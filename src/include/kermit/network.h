#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <cstdint>

namespace kermit {

// Network interface
class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();
    
    // Initialize network
    bool initialize(uint16_t listen_port, const std::string& listen_address = "0.0.0.0");
    
    // Start/stop network operations
    bool start();
    void stop();
    
    // Connection management
    bool connect(const std::string& host, uint16_t port);
    void disconnect(const std::string& connection_id);
    
    // Data transmission
    bool sendData(const std::string& connection_id, const std::vector<uint8_t>& data);
    std::vector<uint8_t> receiveData(const std::string& connection_id);
    
    // Callback registration
    using ConnectionCallback = std::function<void(const std::string&, bool)>;
    using DataCallback = std::function<void(const std::string&, const std::vector<uint8_t>&)>;
    
    void setConnectionCallback(ConnectionCallback callback);
    void setDataCallback(DataCallback callback);
    
    // Network information
    std::vector<std::string> getActiveConnections() const;
    bool isConnected(const std::string& connection_id) const;
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// Relay node representation
class RelayNode {
public:
    RelayNode(const std::string& node_id, const std::string& address, uint16_t port);
    ~RelayNode();
    
    const std::string& getNodeId() const;
    const std::string& getAddress() const;
    uint16_t getPort() const;
    
    bool isTrusted() const;
    void setTrusted(bool trusted);
    
    // Node capabilities
    bool supportsHiddenServices() const;
    bool isExitNode() const;
    bool isGuardNode() const;
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace kermit