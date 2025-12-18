#include "kermit/config.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

namespace kermit {

// RouterConfig implementation
RouterConfig::RouterConfig() 
    : data_directory("./data"),
      listen_address("0.0.0.0"),
      listen_port(9050),
      socks_port(9051),
      control_port(9052),
      enable_logging(true),
      log_file("kermit.log"),
      use_ipv6(false),
      enable_hidden_services(true),
      max_circuits(100),
      circuit_timeout(300) {
    // Default hidden service directories
    hidden_service_directories = {"./services/service1", "./services/service2"};
}

// ConfigManager implementation
class ConfigManager::Impl {
public:
    RouterConfig config;
};

ConfigManager::ConfigManager() : impl_(std::make_unique<Impl>()) {}

ConfigManager::~ConfigManager() = default;

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

void ConfigManager::loadConfig(const std::string& config_file) {
    std::ifstream file(config_file);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open config file " << config_file 
                  << ", using defaults" << std::endl;
        return;
    }
    
    std::cout << "Loading configuration from " << config_file << std::endl;
    
    std::string line;
    std::string current_key;
    std::string current_value;
    bool in_array = false;
    
    while (std::getline(file, line)) {
        // Remove comments
        size_t comment_pos = line.find('#');
        if (comment_pos != std::string::npos) {
            line = line.substr(0, comment_pos);
        }
        
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        if (line.empty()) continue;
        
        // Check if we're starting or continuing an array
        if (line.find('[') != std::string::npos) {
            in_array = true;
            size_t equals_pos = line.find('=');
            if (equals_pos != std::string::npos) {
                current_key = line.substr(0, equals_pos);
                current_key.erase(0, current_key.find_first_not_of(" \t"));
                current_key.erase(current_key.find_last_not_of(" \t") + 1);
                
                // Get the rest of the line after =
                current_value = line.substr(equals_pos + 1);
                current_value.erase(0, current_value.find_first_not_of(" \t"));
                current_value.erase(current_value.find_last_not_of(" \t") + 1);
            }
            continue;
        }
        
        // If we're in an array, accumulate lines until we find the closing bracket
        if (in_array) {
            current_value += " " + line;
            
            // Check if this line ends the array
            if (line.find(']') != std::string::npos) {
                in_array = false;
                
                // Trim and parse the array
                current_value.erase(0, current_value.find_first_not_of(" \t"));
                current_value.erase(current_value.find_last_not_of(" \t") + 1);
                
                std::cout << "Config: " << current_key << " = " << current_value << std::endl;
                parseConfigOption(current_key, current_value);
                
                current_key.clear();
                current_value.clear();
            }
            continue;
        }
        
        // Parse regular key-value pairs
        size_t equals_pos = line.find('=');
        if (equals_pos != std::string::npos) {
            std::string key = line.substr(0, equals_pos);
            std::string value = line.substr(equals_pos + 1);
            
            // Trim whitespace from key and value
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            // Remove quotes if present
            if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.size() - 2);
            }
            
            std::cout << "Config: " << key << " = " << value << std::endl;
            
            // Parse configuration
            parseConfigOption(key, value);
        }
    }
    
    std::cout << "Configuration loaded: " << impl_->config.trusted_relays.size() 
              << " trusted relays found" << std::endl;
}

void ConfigManager::parseConfigOption(const std::string& key, const std::string& value) {
    if (key == "data_directory") {
        impl_->config.data_directory = value;
    } else if (key == "listen_address") {
        impl_->config.listen_address = value;
    } else if (key == "listen_port") {
        impl_->config.listen_port = static_cast<uint16_t>(std::stoi(value));
    } else if (key == "socks_port") {
        impl_->config.socks_port = static_cast<uint16_t>(std::stoi(value));
    } else if (key == "control_port") {
        impl_->config.control_port = static_cast<uint16_t>(std::stoi(value));
    } else if (key == "enable_logging") {
        impl_->config.enable_logging = (value == "true" || value == "True" || value == "1");
    } else if (key == "log_file") {
        impl_->config.log_file = value;
    } else if (key == "use_ipv6") {
        impl_->config.use_ipv6 = (value == "true" || value == "True" || value == "1");
    } else if (key == "enable_hidden_services") {
        impl_->config.enable_hidden_services = (value == "true" || value == "True" || value == "1");
    } else if (key == "max_circuits") {
        impl_->config.max_circuits = static_cast<uint32_t>(std::stoi(value));
    } else if (key == "circuit_timeout") {
        impl_->config.circuit_timeout = static_cast<uint32_t>(std::stoi(value));
    } else if (key == "trusted_relays") {
        // Simple array parsing - expect format like ["host:port", "host:port"]
        parseArrayOption(value, impl_->config.trusted_relays);
    }
    // TODO: Parse other arrays like hidden_service_directories
}

void ConfigManager::parseArrayOption(const std::string& array_value, std::vector<std::string>& target_vector) {
    // Simple array parsing - expect format like ["item1", "item2", "item3"]
    // Remove brackets
    std::string content = array_value;
    if (content.front() == '[') {
        content = content.substr(1);
    }
    if (content.back() == ']') {
        content = content.substr(0, content.length() - 1);
    }
    
    // Trim whitespace
    content.erase(0, content.find_first_not_of(" \t\n\r"));
    content.erase(content.find_last_not_of(" \t\n\r") + 1);
    
    if (content.empty()) {
        return;
    }
    
    // Split by commas
    size_t start = 0;
    size_t end = content.find(',');
    
    while (end != std::string::npos) {
        std::string item = content.substr(start, end - start);
        
        // Trim whitespace and quotes
        item.erase(0, item.find_first_not_of(" \t\n\r"));
        item.erase(item.find_last_not_of(" \t\n\r") + 1);
        
        if (item.front() == '"' && item.back() == '"') {
            item = item.substr(1, item.length() - 2);
        }
        
        if (!item.empty()) {
            target_vector.push_back(item);
        }
        
        start = end + 1;
        end = content.find(',', start);
    }
    
    // Process last item
    if (start < content.length()) {
        std::string item = content.substr(start);
        
        // Trim whitespace and quotes
        item.erase(0, item.find_first_not_of(" \t\n\r"));
        item.erase(item.find_last_not_of(" \t\n\r") + 1);
        
        if (item.front() == '"' && item.back() == '"') {
            item = item.substr(1, item.length() - 2);
        }
        
        if (!item.empty()) {
            target_vector.push_back(item);
        }
    }
}

void ConfigManager::saveConfig(const std::string& config_file) {
    std::ofstream file(config_file);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file for writing: " + config_file);
    }
    
    file << "# Kermit Configuration File\n"
         << "# Generated by Kermit\n\n"
         << "data_directory = \"" << impl_->config.data_directory << "\"\n"
         << "listen_address = \"" << impl_->config.listen_address << "\"\n"
         << "listen_port = " << impl_->config.listen_port << "\n"
         << "socks_port = " << impl_->config.socks_port << "\n"
         << "control_port = " << impl_->config.control_port << "\n"
         << "enable_logging = " << (impl_->config.enable_logging ? "true" : "false") << "\n"
         << "log_file = \"" << impl_->config.log_file << "\"\n"
         << "use_ipv6 = " << (impl_->config.use_ipv6 ? "true" : "false") << "\n"
         << "enable_hidden_services = " << (impl_->config.enable_hidden_services ? "true" : "false") << "\n"
         << "max_circuits = " << impl_->config.max_circuits << "\n"
         << "circuit_timeout = " << impl_->config.circuit_timeout << "\n";
    
    // TODO: Save arrays
}

RouterConfig& ConfigManager::getConfig() {
    return impl_->config;
}

const RouterConfig& ConfigManager::getConfig() const {
    return impl_->config;
}

} // namespace kermit