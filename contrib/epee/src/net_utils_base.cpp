#include "contrib/epee/include/net_utils_base.h"
#include "src/cryptonote_basic/connection_context.h"

namespace epee {
namespace net_utils {

bool network_address::equal(const network_address& other) const {
    const network_address::interface* const self_ = self.get();
    const network_address::interface* const other_self = other.self.get();
    if (self_ == other_self) return true;
    if (!self_ || !other_self) return false;
    if (typeid(*self_) != typeid(*other_self)) return false;
    return self_->equal(*other_self);
}

bool network_address::less(const network_address& other) const {
    const network_address::interface* const self_ = self.get();
    const network_address::interface* const other_self = other.self.get();
    if (self_ == other_self) return false;
    if (!self_ || !other_self) return self == nullptr;
    if (typeid(*self_) != typeid(*other_self)) return false;
    return self_->less(*other_self);
}

bool network_address::is_same_host(const network_address& other) const {
    const network_address::interface* const self_ = self.get();
    const network_address::interface* const other_self = other.self.get();
    if (self_ == other_self) return true;
    if (!self_ || !other_self) return false;
    if (typeid(*self_) != typeid(*other_self)) return false;
    return self_->is_same_host(*other_self);
}

std::string print_connection_context(const connection_context_base& ctx) {
    std::stringstream ss;
    ss << ctx.m_remote_address.str() << " " << ctx.m_connection_id << (ctx.m_is_income ? " INC" : " OUT");
    return ss.str();
}

std::string print_connection_context_short(const connection_context_base& ctx) {
    std::stringstream ss;
    ss << ctx.m_remote_address.str() << (ctx.m_is_income ? " INC" : " OUT");
    return ss.str();
}

} // namespace net_utils
} // namespace epee
