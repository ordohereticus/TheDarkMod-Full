//
// socket_option.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_IP_DETAIL_SOCKET_OPTION_HPP
#define BOOST_ASIO_IP_DETAIL_SOCKET_OPTION_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <cstddef>
#include <cstring>
#include <boost/config.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/ip/address.hpp>
#include <boost/asio/detail/socket_ops.hpp>
#include <boost/asio/detail/socket_types.hpp>

namespace boost {
namespace asio {
namespace ip {
namespace detail {
namespace socket_option {

// Helper template for implementing boolean-based options.
template <int IPv4_Level, int IPv4_Name, int IPv6_Level, int IPv6_Name>
class boolean
{
public:
  // Default constructor.
  boolean()
    : value_(0)
  {
  }

  // Construct with a specific option value.
  boolean(bool value)
    : value_(value ? 1 : 0)
  {
  }

  // Set the value of the boolean.
  void set(bool value)
  {
    value_ = value ? 1 : 0;
  }

  // Get the current value of the boolean.
  bool get() const
  {
    return value_;
  }

  // Get the level of the socket option.
  template <typename Protocol>
  int level(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Level;
    return IPv4_Level;
  }

  // Get the name of the socket option.
  template <typename Protocol>
  int name(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Name;
    return IPv4_Name;
  }

  // Get the address of the boolean data.
  template <typename Protocol>
  int* data(const Protocol&)
  {
    return &value_;
  }

  // Get the address of the boolean data.
  template <typename Protocol>
  const int* data(const Protocol&) const
  {
    return &value_;
  }

  // Get the size of the boolean data.
  template <typename Protocol>
  std::size_t size(const Protocol&) const
  {
    return sizeof(value_);
  }

private:
  int value_;
};

// Helper template for implementing integer options.
template <int IPv4_Level, int IPv4_Name, int IPv6_Level, int IPv6_Name>
class integer
{
public:
  // Default constructor.
  integer()
    : value_(0)
  {
  }

  // Construct with a specific option value.
  integer(int value)
    : value_(value)
  {
  }

  // Set the value of the int option.
  void set(int value)
  {
    value_ = value;
  }

  // Get the current value of the int option.
  int get() const
  {
    return value_;
  }

  // Get the level of the socket option.
  template <typename Protocol>
  int level(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Level;
    return IPv4_Level;
  }

  // Get the name of the socket option.
  template <typename Protocol>
  int name(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Name;
    return IPv4_Name;
  }

  // Get the address of the int data.
  template <typename Protocol>
  int* data(const Protocol&)
  {
    return &value_;
  }

  // Get the address of the int data.
  template <typename Protocol>
  const int* data(const Protocol&) const
  {
    return &value_;
  }

  // Get the size of the int data.
  template <typename Protocol>
  std::size_t size(const Protocol&) const
  {
    return sizeof(value_);
  }

private:
  int value_;
};

// Helper template for implementing ip_mreq-based options.
template <int IPv4_Level, int IPv4_Name, int IPv6_Level, int IPv6_Name>
class multicast_request
{
public:
  // Default constructor.
  multicast_request()
  {
    ipv4_value_.imr_multiaddr.s_addr =
      boost::asio::detail::socket_ops::host_to_network_long(
          boost::asio::ip::address_v4::any().to_ulong());
    ipv4_value_.imr_interface.s_addr =
      boost::asio::detail::socket_ops::host_to_network_long(
          boost::asio::ip::address_v4::any().to_ulong());

    boost::asio::detail::in6_addr_type tmp_addr = IN6ADDR_ANY_INIT;
    ipv6_value_.ipv6mr_multiaddr = tmp_addr;
    ipv6_value_.ipv6mr_interface = 0;
  }

  // Construct with multicast address only.
  multicast_request(const boost::asio::ip::address& multicast_address)
  {
    if (multicast_address.is_v6())
    {
      ipv4_value_.imr_multiaddr.s_addr =
        boost::asio::detail::socket_ops::host_to_network_long(
            boost::asio::ip::address_v4::any().to_ulong());
      ipv4_value_.imr_interface.s_addr =
        boost::asio::detail::socket_ops::host_to_network_long(
            boost::asio::ip::address_v4::any().to_ulong());

      using namespace std; // For memcpy.
      boost::asio::ip::address_v6 ipv6_address = multicast_address.to_v6();
      boost::asio::ip::address_v6::bytes_type bytes = ipv6_address.to_bytes();
      memcpy(ipv6_value_.ipv6mr_multiaddr.s6_addr, bytes.elems, 16);
      ipv6_value_.ipv6mr_interface = 0;
    }
    else
    {
      ipv4_value_.imr_multiaddr.s_addr =
        boost::asio::detail::socket_ops::host_to_network_long(
            multicast_address.to_v4().to_ulong());
      ipv4_value_.imr_interface.s_addr =
        boost::asio::detail::socket_ops::host_to_network_long(
            boost::asio::ip::address_v4::any().to_ulong());

      boost::asio::detail::in6_addr_type tmp_addr = IN6ADDR_ANY_INIT;
      ipv6_value_.ipv6mr_multiaddr = tmp_addr;
      ipv6_value_.ipv6mr_interface = 0;
    }
  }

  // Construct with multicast address and IPv4 address specifying an interface.
  multicast_request(const boost::asio::ip::address_v4& multicast_address,
      const boost::asio::ip::address_v4& network_interface
        = boost::asio::ip::address_v4::any())
  {
    ipv4_value_.imr_multiaddr.s_addr =
      boost::asio::detail::socket_ops::host_to_network_long(
          multicast_address.to_ulong());
    ipv4_value_.imr_interface.s_addr =
      boost::asio::detail::socket_ops::host_to_network_long(
          network_interface.to_ulong());

    boost::asio::detail::in6_addr_type tmp_addr = IN6ADDR_ANY_INIT;
    ipv6_value_.ipv6mr_multiaddr = tmp_addr;
    ipv6_value_.ipv6mr_interface = 0;
  }

  // Construct with multicast address and IPv6 network interface index.
  multicast_request(const boost::asio::ip::address_v6& multicast_address,
      unsigned long network_interface = 0)
  {
    ipv4_value_.imr_multiaddr.s_addr =
      boost::asio::detail::socket_ops::host_to_network_long(
          boost::asio::ip::address_v4::any().to_ulong());
    ipv4_value_.imr_interface.s_addr =
      boost::asio::detail::socket_ops::host_to_network_long(
          boost::asio::ip::address_v4::any().to_ulong());

    using namespace std; // For memcpy.
    boost::asio::ip::address_v6::bytes_type bytes =
      multicast_address.to_bytes();
    memcpy(ipv6_value_.ipv6mr_multiaddr.s6_addr, bytes.elems, 16);
    ipv6_value_.ipv6mr_interface = network_interface;
  }

  // Get the level of the socket option.
  template <typename Protocol>
  int level(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Level;
    return IPv4_Level;
  }

  // Get the name of the socket option.
  template <typename Protocol>
  int name(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Name;
    return IPv4_Name;
  }

  // Get the address of the option data.
  template <typename Protocol>
  void* data(const Protocol& protocol)
  {
    if (protocol.family() == PF_INET6)
      return &ipv6_value_;
    return &ipv4_value_;
  }

  // Get the address of the option data.
  template <typename Protocol>
  const void* data(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return &ipv6_value_;
    return &ipv4_value_;
  }

  // Get the size of the option data.
  template <typename Protocol>
  std::size_t size(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return sizeof(ipv6_value_);
    return sizeof(ipv4_value_);
  }

private:
  boost::asio::detail::in4_mreq_type ipv4_value_;
  boost::asio::detail::in6_mreq_type ipv6_value_;
};

// Helper template for implementing options that specify a network interface.
template <int IPv4_Level, int IPv4_Name, int IPv6_Level, int IPv6_Name>
class network_interface
{
public:
  // Default constructor.
  network_interface()
  {
    ipv4_value_.s_addr =
      boost::asio::detail::socket_ops::host_to_network_long(
          boost::asio::ip::address_v4::any().to_ulong());
    ipv6_value_ = 0;
  }

  // Construct with IPv4 interface.
  network_interface(const boost::asio::ip::address_v4& ipv4_interface)
  {
    ipv4_value_.s_addr =
      boost::asio::detail::socket_ops::host_to_network_long(
          ipv4_interface.to_ulong());
    ipv6_value_ = 0;
  }

  // Construct with IPv6 interface.
  network_interface(unsigned long ipv6_interface)
  {
    ipv4_value_.s_addr =
      boost::asio::detail::socket_ops::host_to_network_long(
          boost::asio::ip::address_v4::any().to_ulong());
    ipv6_value_ = ipv6_interface;
  }

  // Get the level of the socket option.
  template <typename Protocol>
  int level(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Level;
    return IPv4_Level;
  }

  // Get the name of the socket option.
  template <typename Protocol>
  int name(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return IPv6_Name;
    return IPv4_Name;
  }

  // Get the address of the option data.
  template <typename Protocol>
  void* data(const Protocol& protocol)
  {
    if (protocol.family() == PF_INET6)
      return &ipv6_value_;
    return &ipv4_value_;
  }

  // Get the address of the option data.
  template <typename Protocol>
  const void* data(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return &ipv6_value_;
    return &ipv4_value_;
  }

  // Get the size of the option data.
  template <typename Protocol>
  std::size_t size(const Protocol& protocol) const
  {
    if (protocol.family() == PF_INET6)
      return sizeof(ipv6_value_);
    return sizeof(ipv4_value_);
  }

private:
  boost::asio::detail::in4_addr_type ipv4_value_;
  unsigned long ipv6_value_;
};

} // namespace socket_option
} // namespace detail
} // namespace ip
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_IP_DETAIL_SOCKET_OPTION_HPP
