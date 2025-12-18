#include <iostream>
#include "src/include/kermit/config.h"

int main() {
    std::cout << "Testing configuration loading..." << std::endl;
    
    try {
        auto& config_manager = kermit::ConfigManager::getInstance();
        config_manager.loadConfig("simple_test.conf");
        
        const auto& config = config_manager.getConfig();
        
        std::cout << "Configuration loaded successfully!" << std::endl;
        std::cout << "Trusted relays count: " << config.trusted_relays.size() << std::endl;
        
        if (!config.trusted_relays.empty()) {
            std::cout << "Trusted relay nodes:" << std::endl;
            for (const auto& relay : config.trusted_relays) {
                std::cout << "  - " << relay << std::endl;
            }
        }
        
        // Check for our specific node
        bool found = false;
        for (const auto& relay : config.trusted_relays) {
            if (relay.find("31.3.218.22:9001") != std::string::npos) {
                found = true;
                break;
            }
        }
        
        if (found) {
            std::cout << "✓ SUCCESS: First node 31.3.218.22:9001 found!" << std::endl;
            return 0;
        } else {
            std::cout << "✗ FAILED: First node 31.3.218.22:9001 not found" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}