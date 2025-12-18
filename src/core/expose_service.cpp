#include "kermit/expose_service.h"
#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <regex>

namespace kermit {

ServiceRegistry::ServiceRegistry() {}

ServiceRegistry::~ServiceRegistry() {
    std::lock_guard<std::mutex> lock(services_mutex_);
    services_.clear();
}

std::string ServiceRegistry::generateServiceHash() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint8_t> dis(0, 255);

    std::stringstream ss;
    // Generate 6 random bytes (48 bits)
    for (int i = 0; i < 6; ++i) {
        uint8_t byte = dis(gen);
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
    }
    
    // Add .uwu extension
    return ss.str() + ".uwu";
}

bool ServiceRegistry::isValidServiceHash(const std::string& hash) {
    // Format: 12 hex characters followed by ".uwu"
    std::regex pattern("^[0-9a-f]{12}\\.uwu$");
    return std::regex_match(hash, pattern);
}

std::string ServiceRegistry::normalizeAddress(const std::string& address) {
    // Validate ip:port format
    std::regex pattern("^([0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}|localhost|[a-zA-Z0-9.-]+):[0-9]{1,5}$");
    
    if (!std::regex_match(address, pattern)) {
        throw std::invalid_argument("Invalid address format. Expected: ip:port or hostname:port");
    }
    
    return address;
}

std::string ServiceRegistry::exposeService(const std::string& target_address) {
    // Validate address format
    std::string normalized_address = normalizeAddress(target_address);
    
    // Generate unique service hash
    std::string service_hash;
    {
        std::lock_guard<std::mutex> lock(services_mutex_);
        
        // Ensure uniqueness
        do {
            service_hash = generateServiceHash();
        } while (services_.find(service_hash) != services_.end());
        
        // Create service handle
        auto handle = std::make_shared<ServiceHandle>();
        handle->service_hash = service_hash;
        handle->target_address = normalized_address;
        handle->created_timestamp = std::chrono::system_clock::now().time_since_epoch().count();
        handle->is_active = true;
        
        services_[service_hash] = handle;
    }
    
    std::cout << "Service exposed: " << service_hash << " -> " << normalized_address << std::endl;
    return service_hash;
}

std::string ServiceRegistry::resolveService(const std::string& service_hash) {
    if (!isValidServiceHash(service_hash)) {
        return "";
    }
    
    std::lock_guard<std::mutex> lock(services_mutex_);
    
    auto it = services_.find(service_hash);
    if (it != services_.end() && it->second->is_active) {
        return it->second->target_address;
    }
    
    return "";
}

std::shared_ptr<ServiceHandle> ServiceRegistry::getServiceHandle(const std::string& service_hash) {
    if (!isValidServiceHash(service_hash)) {
        return nullptr;
    }
    
    std::lock_guard<std::mutex> lock(services_mutex_);
    
    auto it = services_.find(service_hash);
    if (it != services_.end()) {
        return it->second;
    }
    
    return nullptr;
}

bool ServiceRegistry::revokeService(const std::string& service_hash) {
    if (!isValidServiceHash(service_hash)) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(services_mutex_);
    
    auto it = services_.find(service_hash);
    if (it != services_.end()) {
        it->second->is_active = false;
        services_.erase(it);
        std::cout << "Service revoked: " << service_hash << std::endl;
        return true;
    }
    
    return false;
}

std::vector<std::shared_ptr<ServiceHandle>> ServiceRegistry::listServices() {
    std::lock_guard<std::mutex> lock(services_mutex_);
    
    std::vector<std::shared_ptr<ServiceHandle>> result;
    for (const auto& pair : services_) {
        if (pair.second->is_active) {
            result.push_back(pair.second);
        }
    }
    return result;
}

}  // namespace kermit
