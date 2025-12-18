#include "kermit/node_manager.h"
#include "kermit/network.h"
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <mutex>
#include <random>
#include <sstream>
#include <algorithm>

namespace kermit {

// NodeManager implementation
class NodeManager::Impl {
public:
    std::map<std::string, std::shared_ptr<RelayNode>> nodes_;
    std::map<std::string, bool> connected_nodes_;
    std::mutex nodes_mutex_;
    std::shared_ptr<NetworkManager> network_manager_;
    
    Impl() {
        network_manager_ = std::make_unique<NetworkManager>();
    }
    
    ~Impl() {
        // Disconnect from all nodes
        std::lock_guard<std::mutex> lock(nodes_mutex_);
        for (auto& conn : connected_nodes_) {
            if (conn.second) {
                network_manager_->disconnect(conn.first);
            }
        }
        connected_nodes_.clear();
    }
    
    bool initialize() {
        // Initialize network manager
        if (!network_manager_->initialize(0, "0.0.0.0")) {
            std::cerr << "Failed to initialize network manager" << std::endl;
            return false;
        }
        
        if (!network_manager_->start()) {
            std::cerr << "Failed to start network manager" << std::endl;
            return false;
        }
        
        std::cout << "Node manager initialized" << std::endl;
        return true;
    }
    
    bool addRelayNode(const std::string& node_id, const std::string& address, uint16_t port, bool trusted) {
        std::lock_guard<std::mutex> lock(nodes_mutex_);
        
        // Check if node already exists
        if (nodes_.find(node_id) != nodes_.end()) {
            std::cerr << "Node " << node_id << " already exists" << std::endl;
            return false;
        }
        
        // Create new relay node
        auto node = std::make_shared<RelayNode>(node_id, address, port);
        node->setTrusted(trusted);
        
        nodes_[node_id] = node;
        connected_nodes_[node_id] = false;
        
        std::cout << "Added relay node " << node_id << " at " 
                  << address << ":" << port 
                  << (trusted ? " (trusted)" : "") << std::endl;
        
        return true;
    }
    
    bool addRelayNodeFromString(const std::string& node_address, bool trusted) {
        size_t colon_pos = node_address.find(':');
        if (colon_pos == std::string::npos) {
            std::cerr << "Invalid node address format: " << node_address 
                      << " (expected host:port)" << std::endl;
            return false;
        }
        
        std::string host = node_address.substr(0, colon_pos);
        std::string port_str = node_address.substr(colon_pos + 1);
        
        try {
            uint16_t port = static_cast<uint16_t>(std::stoi(port_str));
            
            // Use host:port as node ID for simplicity
            std::string node_id = host + ":" + port_str;
            
            return addRelayNode(node_id, host, port, trusted);
            
        } catch (const std::exception& e) {
            std::cerr << "Invalid port number: " << port_str << std::endl;
            return false;
        }
    }
    
    bool removeRelayNode(const std::string& node_id) {
        std::lock_guard<std::mutex> lock(nodes_mutex_);
        
        auto node_it = nodes_.find(node_id);
        if (node_it == nodes_.end()) {
            std::cerr << "Node " << node_id << " not found" << std::endl;
            return false;
        }
        
        // Disconnect if connected
        auto conn_it = connected_nodes_.find(node_id);
        if (conn_it != connected_nodes_.end() && conn_it->second) {
            network_manager_->disconnect(node_id);
        }
        
        nodes_.erase(node_it);
        connected_nodes_.erase(conn_it);
        
        std::cout << "Removed relay node " << node_id << std::endl;
        return true;
    }
    
    std::shared_ptr<RelayNode> getRelayNode(const std::string& node_id) {
        std::lock_guard<std::mutex> lock(nodes_mutex_);
        
        auto it = nodes_.find(node_id);
        if (it == nodes_.end()) {
            return nullptr;
        }
        
        return it->second;
    }
    
    std::vector<std::shared_ptr<RelayNode>> getAllRelayNodes() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(nodes_mutex_));
        
        std::vector<std::shared_ptr<RelayNode>> all_nodes;
        for (const auto& node : nodes_) {
            all_nodes.push_back(node.second);
        }
        
        return all_nodes;
    }
    
    std::vector<std::shared_ptr<RelayNode>> getTrustedRelayNodes() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(nodes_mutex_));
        
        std::vector<std::shared_ptr<RelayNode>> trusted_nodes;
        for (const auto& node : nodes_) {
            if (node.second->isTrusted()) {
                trusted_nodes.push_back(node.second);
            }
        }
        
        return trusted_nodes;
    }
    
    std::shared_ptr<RelayNode> getRandomRelayNode() const {
        auto all_nodes = getAllRelayNodes();
        if (all_nodes.empty()) {
            return nullptr;
        }
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, all_nodes.size() - 1);
        
        return all_nodes[dis(gen)];
    }
    
    std::shared_ptr<RelayNode> getRandomTrustedRelayNode() const {
        auto trusted_nodes = getTrustedRelayNodes();
        if (trusted_nodes.empty()) {
            return nullptr;
        }
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, trusted_nodes.size() - 1);
        
        return trusted_nodes[dis(gen)];
    }
    
    size_t getRelayNodeCount() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(nodes_mutex_));
        return nodes_.size();
    }
    
    size_t getTrustedRelayNodeCount() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(nodes_mutex_));
        
        size_t count = 0;
        for (const auto& node : nodes_) {
            if (node.second->isTrusted()) {
                count++;
            }
        }
        
        return count;
    }
    
    bool connectToRelayNode(const std::string& node_id) {
        std::lock_guard<std::mutex> lock(nodes_mutex_);
        
        auto node_it = nodes_.find(node_id);
        if (node_it == nodes_.end()) {
            std::cerr << "Node " << node_id << " not found" << std::endl;
            return false;
        }
        
        auto conn_it = connected_nodes_.find(node_id);
        if (conn_it != connected_nodes_.end() && conn_it->second) {
            std::cout << "Already connected to " << node_id << std::endl;
            return true;
        }
        
        // Connect to the node
        auto node = node_it->second;
        bool success = network_manager_->connect(node->getAddress(), node->getPort());
        
        if (success) {
            connected_nodes_[node_id] = true;
            std::cout << "Connected to relay node " << node_id << std::endl;
        } else {
            std::cerr << "Failed to connect to " << node_id << std::endl;
        }
        
        return success;
    }
    
    void disconnectFromRelayNode(const std::string& node_id) {
        std::lock_guard<std::mutex> lock(nodes_mutex_);
        
        auto conn_it = connected_nodes_.find(node_id);
        if (conn_it == connected_nodes_.end() || !conn_it->second) {
            return;
        }
        
        network_manager_->disconnect(node_id);
        connected_nodes_[node_id] = false;
        
        std::cout << "Disconnected from relay node " << node_id << std::endl;
    }
    
    bool isConnectedToRelayNode(const std::string& node_id) const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(nodes_mutex_));
        
        auto it = connected_nodes_.find(node_id);
        return it != connected_nodes_.end() && it->second;
    }
    
    void loadFromConfig(const std::vector<std::string>& trusted_relays) {
        std::lock_guard<std::mutex> lock(nodes_mutex_);
        
        std::cout << "Loading " << trusted_relays.size() << " trusted relay nodes from config..." << std::endl;
        
        for (const auto& relay_addr : trusted_relays) {
            if (!relay_addr.empty() && relay_addr.find(':') != std::string::npos) {
                addRelayNodeFromString(relay_addr, true);
            }
        }
        
        std::cout << "Loaded " << nodes_.size() << " relay nodes" << std::endl;
    }
};

// NodeManager public interface
NodeManager::NodeManager() : impl_(std::make_unique<Impl>()) {}

NodeManager::~NodeManager() = default;

bool NodeManager::initialize() {
    return impl_->initialize();
}

bool NodeManager::addRelayNode(const std::string& node_id, const std::string& address, uint16_t port, bool trusted) {
    return impl_->addRelayNode(node_id, address, port, trusted);
}

bool NodeManager::addRelayNodeFromString(const std::string& node_address, bool trusted) {
    return impl_->addRelayNodeFromString(node_address, trusted);
}

bool NodeManager::removeRelayNode(const std::string& node_id) {
    return impl_->removeRelayNode(node_id);
}

std::shared_ptr<RelayNode> NodeManager::getRelayNode(const std::string& node_id) {
    return impl_->getRelayNode(node_id);
}

std::vector<std::shared_ptr<RelayNode>> NodeManager::getAllRelayNodes() const {
    return impl_->getAllRelayNodes();
}

std::vector<std::shared_ptr<RelayNode>> NodeManager::getTrustedRelayNodes() const {
    return impl_->getTrustedRelayNodes();
}

std::shared_ptr<RelayNode> NodeManager::getRandomRelayNode() const {
    return impl_->getRandomRelayNode();
}

std::shared_ptr<RelayNode> NodeManager::getRandomTrustedRelayNode() const {
    return impl_->getRandomTrustedRelayNode();
}

size_t NodeManager::getRelayNodeCount() const {
    return impl_->getRelayNodeCount();
}

size_t NodeManager::getTrustedRelayNodeCount() const {
    return impl_->getTrustedRelayNodeCount();
}

bool NodeManager::connectToRelayNode(const std::string& node_id) {
    return impl_->connectToRelayNode(node_id);
}

void NodeManager::disconnectFromRelayNode(const std::string& node_id) {
    impl_->disconnectFromRelayNode(node_id);
}

bool NodeManager::isConnectedToRelayNode(const std::string& node_id) const {
    return impl_->isConnectedToRelayNode(node_id);
}

void NodeManager::loadFromConfig(const std::vector<std::string>& trusted_relays) {
    impl_->loadFromConfig(trusted_relays);
}

} // namespace kermit