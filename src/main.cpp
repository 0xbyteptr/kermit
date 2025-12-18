#include <iostream>
#include <memory>
#include <csignal>
#include <cstdlib>

#include "kermit/config.h"
#include "kermit/core.h"

using namespace kermit;

// Global router instance
std::unique_ptr<Router> g_router;

// Signal handler for graceful shutdown
void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    
    if (g_router && g_router->isRunning()) {
        g_router->stop();
    }
    
    exit(0);
}

void printUsage(const std::string& program_name) {
    std::cout << "Usage: " << program_name << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -c, --config <file>    Use specified config file" << std::endl;
    std::cout << "  -h, --help            Show this help message" << std::endl;
    std::cout << "  -v, --version         Show version information" << std::endl;
    std::cout << std::endl;
    std::cout << "Kermit - Hidden Service Router" << std::endl;
    std::cout << "Copyright (C) 2023 Kermit Developers" << std::endl;
}

int main(int argc, char* argv[]) {
    std::string config_file = "kermit.conf";
    
    // Parse command line arguments
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
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }

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