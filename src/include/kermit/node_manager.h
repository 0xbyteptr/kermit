#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include <map>

namespace kermit {

class RelayNode;

// Node manager for handling relay nodes
class NodeManager {
public:
    NodeManager();
    ~NodeManager();
    
    // Initialize node manager
    bool initialize();
    
    // Add a relay node
    bool addRelayNode(const std::string& node_id, const std::string& address, uint16_t port, bool trusted = false);
    
    // Add a relay node from address string (host:port)
    bool addRelayNodeFromString(const std::string& node_address, bool trusted = false);
    
    // Remove a relay node
    bool removeRelayNode(const std::string& node_id);
    
    // Get relay node by ID
    std::shared_ptr<RelayNode> getRelayNode(const std::string& node_id);
    
    // Get all relay nodes
    std::vector<std::shared_ptr<RelayNode>> getAllRelayNodes() const;
    
    // Get trusted relay nodes
    std::vector<std::shared_ptr<RelayNode>> getTrustedRelayNodes() const;
    
    // Get random relay node
    std::shared_ptr<RelayNode> getRandomRelayNode() const;
    
    // Get random trusted relay node
    std::shared_ptr<RelayNode> getRandomTrustedRelayNode() const;
    
    // Get number of relay nodes
    size_t getRelayNodeCount() const;
    size_t getTrustedRelayNodeCount() const;
    
    // Connect to a relay node
    bool connectToRelayNode(const std::string& node_id);
    
    // Disconnect from a relay node
    void disconnectFromRelayNode(const std::string& node_id);
    
    // Check if connected to a relay node
    bool isConnectedToRelayNode(const std::string& node_id) const;
    
    // Load nodes from configuration
    void loadFromConfig(const std::vector<std::string>& trusted_relays);
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace kermit