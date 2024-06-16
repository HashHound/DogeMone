// Copyright (c) 2006-2013, Andrey N. Sabelnikov, www.sabelnikov.net
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
// * Neither the name of the Andrey N. Sabelnikov nor the
// names of its contributors may be used to endorse or promote products
// derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER  BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//



#ifndef _NET_UTILS_BASE_H_
#define _NET_UTILS_BASE_H_

#include <boost/uuid/uuid.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/address_v6.hpp>
#include <typeinfo>
#include <type_traits>
#include "byte_slice.h"
#include "enums.h"
#include "misc_log_ex.h"
#include "serialization/keyvalue_serialization.h"
#include "int-util.h"

#undef MONERO_DEFAULT_LOG_CATEGORY
#define MONERO_DEFAULT_LOG_CATEGORY "net"

#ifndef MAKE_IP
#define MAKE_IP( a1, a2, a3, a4 )    (a1|(a2<<8)|(a3<<16)|(((uint32_t)a4)<<24))
#endif

#if BOOST_VERSION >= 107000
#define GET_IO_SERVICE(s) ((boost::asio::io_context&)(s).get_executor().context())
#else
#define GET_IO_SERVICE(s) ((s).get_io_service())
#endif

// Add SWAP32LE function declaration if not already included
#ifndef SWAP32LE
#define SWAP32LE(val) ((((val) & 0xFF000000) >> 24) | (((val) & 0x00FF0000) >> 8) | (((val) & 0x0000FF00) << 8) | (((val) & 0x000000FF) << 24))
#endif

namespace net
{
    class tor_address;
    class i2p_address;
}

namespace epee
{
namespace net_utils
{
    class ipv4_network_address
    {
        uint32_t m_ip;
        uint16_t m_port;

    public:
        constexpr ipv4_network_address() noexcept
            : ipv4_network_address(0, 0)
        {}

        constexpr ipv4_network_address(uint32_t ip, uint16_t port) noexcept
            : m_ip(ip), m_port(port) {}

        bool equal(const ipv4_network_address& other) const noexcept;
        bool less(const ipv4_network_address& other) const noexcept;
        constexpr bool is_same_host(const ipv4_network_address& other) const noexcept
        { return ip() == other.ip(); }

        constexpr uint32_t ip() const noexcept { return m_ip; }
        constexpr uint16_t port() const noexcept { return m_port; }
        std::string str() const;
        std::string host_str() const;
        bool is_loopback() const;
        bool is_local() const;
        static constexpr address_type get_type_id() noexcept { return address_type::ipv4; }
        static constexpr zone get_zone() noexcept { return zone::public_; }
        static constexpr bool is_blockable() noexcept { return true; }

        BEGIN_KV_SERIALIZE_MAP()
            if (is_store)
            {
                uint32_t ip = SWAP32LE(this_ref.m_ip);
                epee::serialization::selector<is_store>::serialize(ip, stg, hparent_section, "m_ip");
            }
            else
            {
                KV_SERIALIZE(m_ip)
                const_cast<ipv4_network_address&>(this_ref).m_ip = SWAP32LE(this_ref.m_ip);
            }
            KV_SERIALIZE(m_port)
        END_KV_SERIALIZE_MAP()
    };

    inline bool operator==(const ipv4_network_address& lhs, const ipv4_network_address& rhs) noexcept
    { return lhs.equal(rhs); }
    inline bool operator!=(const ipv4_network_address& lhs, const ipv4_network_address& rhs) noexcept
    { return !lhs.equal(rhs); }
    inline bool operator<(const ipv4_network_address& lhs, const ipv4_network_address& rhs) noexcept
    { return lhs.less(rhs); }
    inline bool operator<=(const ipv4_network_address& lhs, const ipv4_network_address& rhs) noexcept
    { return !rhs.less(lhs); }
    inline bool operator>(const ipv4_network_address& lhs, const ipv4_network_address& rhs) noexcept
    { return rhs.less(lhs); }
    inline bool operator>=(const ipv4_network_address& lhs, const ipv4_network_address& rhs) noexcept
    { return !lhs.less(rhs); }

    class ipv4_network_subnet
    {
        uint32_t m_ip;
        uint8_t m_mask;

    public:
        constexpr ipv4_network_subnet() noexcept
            : ipv4_network_subnet(0, 0)
        {}

        constexpr ipv4_network_subnet(uint32_t ip, uint8_t mask) noexcept
            : m_ip(ip), m_mask(mask) {}

        bool equal(const ipv4_network_subnet& other) const noexcept;
        bool less(const ipv4_network_subnet& other) const noexcept;
        constexpr bool is_same_host(const ipv4_network_subnet& other) const noexcept
        { return subnet() == other.subnet(); }
                bool matches(const ipv4_network_address &address) const;

        constexpr uint32_t subnet() const noexcept { return m_ip & ~(0xffffffffull << m_mask); }
        std::string str() const;
        std::string host_str() const;
        bool is_loopback() const;
        bool is_local() const;
        static constexpr address_type get_type_id() noexcept { return address_type::invalid; }
        static constexpr zone get_zone() noexcept { return zone::public_; }
        static constexpr bool is_blockable() noexcept { return true; }

        BEGIN_KV_SERIALIZE_MAP()
            KV_SERIALIZE(m_ip)
            KV_SERIALIZE(m_mask)
        END_KV_SERIALIZE_MAP()
    };

    inline bool operator==(const ipv4_network_subnet& lhs, const ipv4_network_subnet& rhs) noexcept
    { return lhs.equal(rhs); }
    inline bool operator!=(const ipv4_network_subnet& lhs, const ipv4_network_subnet& rhs) noexcept
    { return !lhs.equal(rhs); }
    inline bool operator<(const ipv4_network_subnet& lhs, const ipv4_network_subnet& rhs) noexcept
    { return lhs.less(rhs); }
    inline bool operator<=(const ipv4_network_subnet& lhs, const ipv4_network_subnet& rhs) noexcept
    { return !rhs.less(lhs); }
    inline bool operator>(const ipv4_network_subnet& lhs, const ipv4_network_subnet& rhs) noexcept
    { return rhs.less(lhs); }
    inline bool operator>=(const ipv4_network_subnet& lhs, const ipv4_network_subnet& rhs) noexcept
    { return !lhs.less(rhs); }

    class ipv6_network_address
    {
    protected:
        boost::asio::ip::address_v6 m_address;
        uint16_t m_port;

    public:
        ipv6_network_address()
            : ipv6_network_address(boost::asio::ip::address_v6::loopback(), 0)
        {}

        ipv6_network_address(const boost::asio::ip::address_v6& ip, uint16_t port)
            : m_address(ip), m_port(port)
        {
        }

        bool equal(const ipv6_network_address& other) const noexcept;
        bool less(const ipv6_network_address& other) const noexcept;
        bool is_same_host(const ipv6_network_address& other) const noexcept
        { return m_address == other.m_address; }

        boost::asio::ip::address_v6 ip() const noexcept { return m_address; }
        uint16_t port() const noexcept { return m_port; }
        std::string str() const;
        std::string host_str() const;
        bool is_loopback() const;
        bool is_local() const;
        static constexpr address_type get_type_id() noexcept { return address_type::ipv6; }
        static constexpr zone get_zone() noexcept { return zone::public_; }
        static constexpr bool is_blockable() noexcept { return true; }

        static const uint8_t ID = 2;
        BEGIN_KV_SERIALIZE_MAP()
            boost::asio::ip::address_v6::bytes_type bytes = this_ref.m_address.to_bytes();
            epee::serialization::selector<is_store>::serialize_t_val_as_blob(bytes, stg, hparent_section, "addr");
            const_cast<boost::asio::ip::address_v6&>(this_ref.m_address) = boost::asio::ip::address_v6(bytes);
            KV_SERIALIZE(m_port)
        END_KV_SERIALIZE_MAP()
    };

    inline bool operator==(const ipv6_network_address& lhs, const ipv6_network_address& rhs) noexcept
    { return lhs.equal(rhs); }
    inline bool operator!=(const ipv6_network_address& lhs, const ipv6_network_address& rhs) noexcept
    { return !lhs.equal(rhs); }
    inline bool operator<(const ipv6_network_address& lhs, const ipv6_network_address& rhs) noexcept
    { return lhs.less(rhs); }
    inline bool operator<=(const ipv6_network_address& lhs, const ipv6_network_address& rhs) noexcept
    { return !rhs.less(lhs); }
    inline bool operator>(const ipv6_network_address& lhs, const ipv6_network_address& rhs) noexcept
    { return rhs.less(lhs); }
    inline bool operator>=(const ipv6_network_address& lhs, const ipv6_network_address& rhs) noexcept
    { return !lhs.less(rhs); }
}
}

#endif // _NET_UTILS_BASE_H_
