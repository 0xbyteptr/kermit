#pragma once

#include <string>
#include <memory>
#include <vector>
#include <cstdint>

namespace kermit {

// Forward declarations
class Circuit;
class Router;
class HiddenService;
class RelayNode;

// Core router interface
class Router {
public:
    Router();
    virtual ~Router();
    
    // Initialize the router
    bool initialize(const std::string& config_file = "");
    
    // Start the router
    bool start();
    
    // Stop the router
    void stop();
    
    // Main event loop
    void run();
    
    // Circuit management
    std::shared_ptr<Circuit> createCircuit();
    void destroyCircuit(std::shared_ptr<Circuit> circuit);
    
    // Hidden service management
    bool addHiddenService(const std::string& service_dir);
    bool removeHiddenService(const std::string& service_dir);
    
    // Network operations
    bool connectToNetwork();
    void disconnectFromNetwork();
    
    // Status information
    bool isRunning() const;
    size_t getCircuitCount() const;
    size_t getHiddenServiceCount() const;
    
    // Node management
    size_t getRelayNodeCount() const;
    size_t getTrustedRelayNodeCount() const;
    bool addRelayNode(const std::string& node_address, bool trusted);
    bool connectToRelayNode(const std::string& node_id);
    
private:
    // Private implementation details
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// Circuit class for managing connections
class Circuit {
public:
    enum class CircuitState {
        NEW,
        BUILDING,
        ESTABLISHED,
        FAILED,
        CLOSED
    };
    
    Circuit();
    virtual ~Circuit();
    
    // Circuit operations
    bool extend(const std::string& node_id);
    bool sendData(const std::vector<uint8_t>& data);
    std::vector<uint8_t> receiveData();
    
    // Circuit information
    CircuitState getState() const;
    size_t getHopCount() const;
    const std::string& getCircuitId() const;
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// Hidden service representation
class HiddenService {
public:
    HiddenService(const std::string& service_dir);
    ~HiddenService();
    
    bool initialize();
    bool start();
    void stop();
    
    const std::string& getServiceId() const;
    const std::string& getPrivateKey() const;
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace kermit