#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace kermit {

// Configuration structure for Kermit router
struct RouterConfig {
    std::string data_directory;
    std::string listen_address;
    uint16_t listen_port;
    uint16_t socks_port;
    uint16_t control_port;
    bool enable_logging;
    std::string log_file;
    bool use_ipv6;
    
    // Hidden service configuration
    std::vector<std::string> hidden_service_directories;
    bool enable_hidden_services;
    
    // Network configuration
    std::vector<std::string> trusted_relays;
    uint32_t max_circuits;
    uint32_t circuit_timeout;
    
    // Default constructor with sensible defaults
    RouterConfig();
};

// Global configuration access
class ConfigManager {
public:
    static ConfigManager& getInstance();
    
    void loadConfig(const std::string& config_file);
    void saveConfig(const std::string& config_file);
    
    RouterConfig& getConfig();
    const RouterConfig& getConfig() const;
    
private:
    void parseConfigOption(const std::string& key, const std::string& value);
    void parseArrayOption(const std::string& array_value, std::vector<std::string>& target_vector);
    
    class Impl;
    std::unique_ptr<Impl> impl_;
    
private:
    ConfigManager();
    ~ConfigManager();
    
    RouterConfig config_;
};

} // namespace kermit