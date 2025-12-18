#pragma once

#include <string>
#include <map>
#include <memory>
#include <cstdint>
#include <vector>
#include <mutex>

namespace kermit {

// Service handle for accessing exposed services
struct ServiceHandle {
    std::string service_hash;  // Random hash like "a1b2c3d4e5f6.uwu"
    std::string target_address;  // Original ip:port
    uint64_t created_timestamp;
    bool is_active;
};

// Service registry for managing exposed hidden services
class ServiceRegistry {
public:
    ServiceRegistry();
    virtual ~ServiceRegistry();

    // Register a new exposed service
    // Returns the service hash (e.g., "a1b2c3d4e5f6.uwu")
    std::string exposeService(const std::string& target_address);

    // Resolve a service hash to its target address
    // Returns empty string if service not found
    std::string resolveService(const std::string& service_hash);

    // Lookup service information
    std::shared_ptr<ServiceHandle> getServiceHandle(const std::string& service_hash);

    // Revoke/remove an exposed service
    bool revokeService(const std::string& service_hash);

    // Get list of all exposed services
    std::vector<std::shared_ptr<ServiceHandle>> listServices();

    // Generate random service hash
    static std::string generateServiceHash();

    // Validate service hash format
    static bool isValidServiceHash(const std::string& hash);

private:
    std::map<std::string, std::shared_ptr<ServiceHandle>> services_;
    mutable std::mutex services_mutex_;

    // Convert ip:port string to hash-friendly format
    std::string normalizeAddress(const std::string& address);
};

}  // namespace kermit
