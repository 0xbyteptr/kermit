#include <iostream>
#include <memory>
#include <csignal>
#include <cstdlib>

#include "kermit/config.h"
#include "kermit/core.h"
#include "kermit/expose_service.h"

using namespace kermit;

// Global router instance
std::unique_ptr<Router> g_router;

// Global service registry
std::unique_ptr<ServiceRegistry> g_service_registry;

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    
    if (g_router && g_router->isRunning()) {
        g_router->stop();
    }
    
    exit(0);
}

void printUsage(const std::string& program_name) {
    std::cout << "Usage: " << program_name << " [command] [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  expose <ip:port>      Expose a service and return .uwu address" << std::endl;
    std::cout << "  revoke <hash>         Revoke an exposed service" << std::endl;
    std::cout << "  list                  List all exposed services" << std::endl;
    std::cout << "  resolve <hash>        Resolve a .uwu address to target" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -c, --config <file>   Use specified config file" << std::endl;
    std::cout << "  -h, --help            Show this help message" << std::endl;
    std::cout << "  -v, --version         Show version information" << std::endl;
    std::cout << std::endl;
    std::cout << "Kermit - Hidden Service Router" << std::endl;
    std::cout << "Copyright (C) 2023 Kermit Developers" << std::endl;
}

int handleExposeCommand(const std::string& target_address) {
    if (!g_service_registry) {
        g_service_registry = std::make_unique<ServiceRegistry>();
    }
    
    try {
        std::string service_hash = g_service_registry->exposeService(target_address);
        std::cout << service_hash << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

int handleRevokeCommand(const std::string& service_hash) {
    if (!g_service_registry) {
        std::cerr << "Error: No services exposed" << std::endl;
        return 1;
    }
    
    if (g_service_registry->revokeService(service_hash)) {
        return 0;
    } else {
        std::cerr << "Error: Service not found" << std::endl;
        return 1;
    }
}

int handleListCommand() {
    if (!g_service_registry) {
        std::cout << "No services exposed" << std::endl;
        return 0;
    }
    
    auto services = g_service_registry->listServices();
    if (services.empty()) {
        std::cout << "No services exposed" << std::endl;
        return 0;
    }
    
    std::cout << "Exposed Services:" << std::endl;
    for (const auto& service : services) {
        std::cout << "  " << service->service_hash << " -> " << service->target_address << std::endl;
    }
    return 0;
}

int handleResolveCommand(const std::string& service_hash) {
    if (!g_service_registry) {
        std::cerr << "Error: No services exposed" << std::endl;
        return 1;
    }
    
    std::string target = g_service_registry->resolveService(service_hash);
    if (!target.empty()) {
        std::cout << target << std::endl;
        return 0;
    } else {
        std::cerr << "Error: Service not found" << std::endl;
        return 1;
    }
}

int main(int argc, char* argv[]) {
    std::string config_file = "kermit.conf";
    
    // Check if command was provided
    if (argc > 1) {
        std::string command = argv[1];
        
        if (command == "expose" && argc > 2) {
            return handleExposeCommand(argv[2]);
        } else if (command == "revoke" && argc > 2) {
            return handleRevokeCommand(argv[2]);
        } else if (command == "list") {
            return handleListCommand();
        } else if (command == "resolve" && argc > 2) {
            return handleResolveCommand(argv[2]);
        } else if (command == "-h" || command == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (command == "-v" || command == "--version") {
            std::cout << "Kermit Hidden Service Router v1.0.0" << std::endl;
            return 0;
        }
    }
    
    // Parse daemon mode arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "-c" || arg == "--config") {
            if (i + 1 < argc) {
                config_file = argv[++i];
            } else {
                std::cerr << "Error: --config requires a file path" << std::endl;
                return 1;
            }
        } else if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if (arg == "-v" || arg == "--version") {
            std::cout << "Kermit Hidden Service Router v1.0.0" << std::endl;
            return 0;
        } else if (arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    // Initialize service registry for daemon mode
    g_service_registry = std::make_unique<ServiceRegistry>();

    // Set up signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    std::cout << "Kermit - Hidden Service Router" << std::endl;
    std::cout << "Starting up..." << std::endl;

    try {
        // Create router instance
        g_router = std::make_unique<Router>();
        
        // Initialize router
        if (!g_router->initialize(config_file)) {
            std::cerr << "Failed to initialize router" << std::endl;
            return 1;
        }
        
        // Start router
        if (!g_router->start()) {
            std::cerr << "Failed to start router" << std::endl;
            return 1;
        }
        
        std::cout << "Router started successfully" << std::endl;
        std::cout << "Press Ctrl+C to shutdown..." << std::endl;
        
        // Run main event loop
        g_router->run();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Router shutdown complete" << std::endl;
    return 0;
}