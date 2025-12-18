#include <iostream>
#include <memory>
#include "src/include/kermit/config.h"
#include "src/include/kermit/core.h"
#include "src/include/kermit/network.h"
#include "src/include/kermit/crypto.h"

int main() {
    std::cout << "Testing Kermit basic functionality..." << std::endl;
    
    // Test configuration
    std::cout << "\n1. Testing Configuration Manager:" << std::endl;
    try {
        auto& config_manager = kermit::ConfigManager::getInstance();
        config_manager.loadConfig("kermit.conf");
        
        const auto& config = config_manager.getConfig();
        std::cout << "   Data directory: " << config.data_directory << std::endl;
        std::cout << "   Listen port: " << config.listen_port << std::endl;
        std::cout << "   SOCKS port: " << config.socks_port << std::endl;
        std::cout << "   Max circuits: " << config.max_circuits << std::endl;
        std::cout << "   Configuration test: PASSED" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "   Configuration test: FAILED - " << e.what() << std::endl;
        return 1;
    }
    
    // Test network manager
    std::cout << "\n2. Testing Network Manager:" << std::endl;
    try {
        kermit::NetworkManager network_manager;
        if (network_manager.initialize(9050, "127.0.0.1")) {
            std::cout << "   Network manager initialized: PASSED" << std::endl;
        } else {
            std::cerr << "   Network manager initialization: FAILED" << std::endl;
            return 1;
        }
        
        if (network_manager.start()) {
            std::cout << "   Network manager started: PASSED" << std::endl;
        } else {
            std::cerr << "   Network manager start: FAILED" << std::endl;
            return 1;
        }
        
        // Test connection (simulated)
        if (network_manager.connect("127.0.0.1", 9051)) {
            std::cout << "   Network connection: PASSED" << std::endl;
        } else {
            std::cerr << "   Network connection: FAILED" << std::endl;
            return 1;
        }
        
        auto connections = network_manager.getActiveConnections();
        std::cout << "   Active connections: " << connections.size() << std::endl;
        
        network_manager.stop();
        std::cout << "   Network manager stopped: PASSED" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "   Network manager test: FAILED - " << e.what() << std::endl;
        return 1;
    }
    
    // Test crypto manager
    std::cout << "\n3. Testing Crypto Manager:" << std::endl;
    try {
        kermit::CryptoManager crypto_manager;
        
        // Test RSA key generation
        std::string rsa_keys = crypto_manager.generateRSAKeyPair();
        if (!rsa_keys.empty() && rsa_keys.find("RSA_KEY_PAIR:") != std::string::npos) {
            std::cout << "   RSA key generation: PASSED" << std::endl;
        } else {
            std::cerr << "   RSA key generation: FAILED" << std::endl;
            return 1;
        }
        
        // Test AES key generation
        std::string aes_key = crypto_manager.generateAESKey();
        if (!aes_key.empty() && aes_key.length() == 64) { // 256-bit key in hex
            std::cout << "   AES key generation: PASSED" << std::endl;
        } else {
            std::cerr << "   AES key generation: FAILED" << std::endl;
            return 1;
        }
        
        // Test SHA256 hashing
        std::vector<uint8_t> test_data = {'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'};
        std::string hash = crypto_manager.hashSHA256(test_data);
        if (!hash.empty() && hash.length() == 64) { // SHA256 hash in hex
            std::cout << "   SHA256 hashing: PASSED (" << hash << ")" << std::endl;
        } else {
            std::cerr << "   SHA256 hashing: FAILED" << std::endl;
            return 1;
        }
        
        // Test random byte generation
        auto random_bytes = crypto_manager.generateRandomBytes(16);
        if (random_bytes.size() == 16) {
            std::cout << "   Random byte generation: PASSED" << std::endl;
        } else {
            std::cerr << "   Random byte generation: FAILED" << std::endl;
            return 1;
        }
        
        std::cout << "   Crypto manager test: PASSED" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "   Crypto manager test: FAILED - " << e.what() << std::endl;
        return 1;
    }
    
    // Test circuit
    std::cout << "\n4. Testing Circuit:" << std::endl;
    try {
        kermit::Circuit circuit;
        
        std::cout << "   Circuit ID: " << circuit.getCircuitId() << std::endl;
        std::cout << "   Initial state: " << static_cast<int>(circuit.getState()) << std::endl;
        
        if (circuit.extend("node1")) {
            std::cout << "   Circuit extension: PASSED" << std::endl;
        } else {
            std::cerr << "   Circuit extension: FAILED" << std::endl;
            return 1;
        }
        
        std::cout << "   Hop count: " << circuit.getHopCount() << std::endl;
        std::cout << "   Circuit test: PASSED" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "   Circuit test: FAILED - " << e.what() << std::endl;
        return 1;
    }
    
    // Test relay node
    std::cout << "\n5. Testing Relay Node:" << std::endl;
    try {
        kermit::RelayNode node("node1", "127.0.0.1", 9050);
        
        std::cout << "   Node ID: " << node.getNodeId() << std::endl;
        std::cout << "   Address: " << node.getAddress() << std::endl;
        std::cout << "   Port: " << node.getPort() << std::endl;
        std::cout << "   Trusted: " << (node.isTrusted() ? "yes" : "no") << std::endl;
        
        node.setTrusted(true);
        std::cout << "   After setting trusted: " << (node.isTrusted() ? "yes" : "no") << std::endl;
        
        std::cout << "   Relay node test: PASSED" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "   Relay node test: FAILED - " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\nAll tests completed successfully!" << std::endl;
    return 0;
}