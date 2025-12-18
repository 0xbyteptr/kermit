#include "kermit/network.h"
#include <iostream>
#include <memory>
#include <cstdint>

namespace kermit {

// RelayNode implementation
class RelayNode::Impl {
public:
    std::string node_id_;
    std::string address_;
    uint16_t port_;
    bool trusted_;
    bool supports_hidden_services_;
    bool is_exit_node_;
    bool is_guard_node_;
    
    Impl(const std::string& node_id, const std::string& address, uint16_t port)
        : node_id_(node_id), address_(address), port_(port), 
          trusted_(false), supports_hidden_services_(true), 
          is_exit_node_(false), is_guard_node_(false) {}
    
    ~Impl() = default;
};

// RelayNode public interface
RelayNode::RelayNode(const std::string& node_id, const std::string& address, uint16_t port)
    : impl_(std::make_unique<Impl>(node_id, address, port)) {}

RelayNode::~RelayNode() = default;

const std::string& RelayNode::getNodeId() const {
    return impl_->node_id_;
}

const std::string& RelayNode::getAddress() const {
    return impl_->address_;
}

uint16_t RelayNode::getPort() const {
    return impl_->port_;
}

bool RelayNode::isTrusted() const {
    return impl_->trusted_;
}

void RelayNode::setTrusted(bool trusted) {
    impl_->trusted_ = trusted;
}

bool RelayNode::supportsHiddenServices() const {
    return impl_->supports_hidden_services_;
}

bool RelayNode::isExitNode() const {
    return impl_->is_exit_node_;
}

bool RelayNode::isGuardNode() const {
    return impl_->is_guard_node_;
}

} // namespace kermit