#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace kermit {

// Cryptographic operations
class CryptoManager {
public:
    CryptoManager();
    ~CryptoManager();
    
    // Key generation
    std::string generateRSAKeyPair();
    std::string generateECDHKeyPair();
    std::string generateAESKey();
    
    // Encryption/Decryption
    std::vector<uint8_t> encryptAES(const std::vector<uint8_t>& data, const std::string& key, const std::string& iv);
    std::vector<uint8_t> decryptAES(const std::vector<uint8_t>& data, const std::string& key, const std::string& iv);
    
    std::vector<uint8_t> encryptRSA(const std::vector<uint8_t>& data, const std::string& public_key);
    std::vector<uint8_t> decryptRSA(const std::vector<uint8_t>& data, const std::string& private_key);
    
    // Hashing
    std::string hashSHA256(const std::vector<uint8_t>& data);
    std::string hashSHA3(const std::vector<uint8_t>& data);
    
    // Digital signatures
    std::string signData(const std::vector<uint8_t>& data, const std::string& private_key);
    bool verifySignature(const std::vector<uint8_t>& data, const std::string& signature, const std::string& public_key);
    
    // Key derivation
    std::string deriveKey(const std::string& secret, const std::string& salt, size_t iterations);
    
    // Random data generation
    std::vector<uint8_t> generateRandomBytes(size_t length);
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

// Onion encryption/decryption for circuit layers
class OnionCrypto {
public:
    OnionCrypto();
    ~OnionCrypto();
    
    // Create onion layers for circuit
    std::vector<uint8_t> createOnionLayers(const std::vector<std::string>& node_keys, const std::vector<uint8_t>& payload);
    
    // Peel onion layer
    std::vector<uint8_t> peelOnionLayer(const std::vector<uint8_t>& onion_data, const std::string& private_key);
    
private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace kermit