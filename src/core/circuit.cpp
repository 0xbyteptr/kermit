#include <kermit/core.h>
#include <iostream>
#include <memory>
#include <vector>
#include <random>
#include <sstream>
#include <iomanip>

namespace kermit {

// Circuit implementation
class Circuit::Impl {
public:
    CircuitState state_;
    std::string circuit_id_;
    std::vector<std::string> nodes_;
    
    Impl() : state_(CircuitState::NEW) {
        // Generate a random circuit ID
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);
        
        std::stringstream ss;
        for (int i = 0; i < 16; ++i) {
            int v = dis(gen);
            ss << std::hex << v;
        }
        
        circuit_id_ = ss.str();
    }
    
    ~Impl() = default;
    
    bool extend(const std::string& node_id) {
        if (state_ == CircuitState::CLOSED || state_ == CircuitState::FAILED) {
            std::cerr << "Cannot extend closed or failed circuit" << std::endl;
            return false;
        }
        
        nodes_.push_back(node_id);
        
        if (state_ == CircuitState::NEW) {
            state_ = CircuitState::BUILDING;
        }
        
        std::cout << "Extended circuit " << circuit_id_ << " with node " << node_id 
                  << " (hop count: " << nodes_.size() << ")" << std::endl;
        
        return true;
    }
    
    bool sendData(const std::vector<uint8_t>& data) {
        if (state_ != CircuitState::ESTABLISHED) {
            std::cerr << "Cannot send data on non-established circuit" << std::endl;
            return false;
        }
        
        std::cout << "Sending " << data.size() << " bytes through circuit " << circuit_id_ 
                  << " (" << nodes_.size() << " hops)" << std::endl;
        
        // TODO: Actual data sending through the circuit
        return true;
    }
    
    std::vector<uint8_t> receiveData() {
        if (state_ != CircuitState::ESTABLISHED) {
            std::cerr << "Cannot receive data on non-established circuit" << std::endl;
            return {};
        }
        
        std::cout << "Receiving data from circuit " << circuit_id_ << std::endl;
        
        // TODO: Actual data receiving
        return {};
    }
    
    CircuitState getState() const {
        return state_;
    }
    
    size_t getHopCount() const {
        return nodes_.size();
    }
    
    const std::string& getCircuitId() const {
        return circuit_id_;
    }
    
    void setState(CircuitState state) {
        state_ = state;
    }
};

// Circuit public interface
Circuit::Circuit() : impl_(std::make_unique<Impl>()) {}

Circuit::~Circuit() = default;

bool Circuit::extend(const std::string& node_id) {
    return impl_->extend(node_id);
}

bool Circuit::sendData(const std::vector<uint8_t>& data) {
    return impl_->sendData(data);
}

std::vector<uint8_t> Circuit::receiveData() {
    return impl_->receiveData();
}

Circuit::CircuitState Circuit::getState() const {
    return impl_->getState();
}

size_t Circuit::getHopCount() const {
    return impl_->getHopCount();
}

const std::string& Circuit::getCircuitId() const {
    return impl_->getCircuitId();
}

} // namespace kermit