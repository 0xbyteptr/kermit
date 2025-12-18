# Kermit - Hidden Service Router

Kermit is a C++ implementation of a hidden service router, similar to Tor's hidden service functionality. It provides anonymous routing and hidden service capabilities.

## Features

- **Configuration Management**: Load and save router configuration
- **Network Management**: TCP/IP network connections with connection management
- **Circuit Management**: Create and manage anonymous circuits through relay nodes
- **Cryptography**: RSA, AES, SHA256, and random data generation
- **Hidden Service Support**: Framework for hidden service management
- **Graceful Shutdown**: Proper signal handling for clean shutdown

## Build Requirements

- C++17 compiler (GCC, Clang, or MSVC)
- CMake 3.10+
- OpenSSL development libraries
- Ninja or Make build system

## Building

```bash
# Clone the repository
git clone https://github.com/yourusername/kermit.git
cd kermit

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
ninja  # or make
```

## Usage

```bash
# Show help
./kermit --help

# Show version
./kermit --version

# Run with default configuration
./kermit

# Run with custom configuration
./kermit --config myconfig.conf
```

## Configuration

Kermit uses a simple configuration file format. See `kermit.conf` for an example configuration.

## Architecture

### Core Components

1. **Router**: Main router class that manages the overall system
2. **NetworkManager**: Handles network connections and data transmission
3. **Circuit**: Manages anonymous circuits through relay nodes
4. **RelayNode**: Represents individual relay nodes in the network
5. **CryptoManager**: Provides cryptographic operations for encryption and authentication
6. **ConfigManager**: Manages configuration loading and saving

### Network Flow

1. Router initializes and loads configuration
2. Network manager starts listening on configured ports
3. Circuits are created and extended through relay nodes
4. Data is encrypted and sent through circuits
5. Hidden services can be added and managed

## Current Status

This is a basic implementation with the core framework in place. The following features are implemented:

- ✅ Configuration management
- ✅ Basic network management
- ✅ Circuit creation and management
- ✅ Cryptographic operations (RSA, AES, SHA256)
- ✅ Relay node management
- ✅ Graceful shutdown handling

## Future Development

The following features are planned for future development:

- [ ] Actual network socket implementation
- [ ] Complete onion routing with layered encryption
- [ ] Hidden service directory management
- [ ] Rendezvous point implementation
- [ ] SOCKS proxy interface
- [ ] Control port interface
- [ ] Performance optimization
- [ ] Security hardening

## License

Kermit is released under the MIT License. See LICENSE for details.

## Contributing

Contributions are welcome! Please submit pull requests or open issues to discuss new features or bug fixes.

## Contact

For questions or support, please open an issue on the GitHub repository.