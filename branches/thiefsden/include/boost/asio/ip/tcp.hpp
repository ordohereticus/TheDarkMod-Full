//
// tcp.hpp
// ~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_IP_TCP_HPP
#define BOOST_ASIO_IP_TCP_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/basic_resolver.hpp>
#include <boost/asio/basic_socket_acceptor.hpp>
#include <boost/asio/basic_socket_iostream.hpp>
#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/ip/basic_endpoint.hpp>
#include <boost/asio/ip/basic_resolver_iterator.hpp>
#include <boost/asio/ip/basic_resolver_query.hpp>
#include <boost/asio/detail/socket_option.hpp>
#include <boost/asio/detail/socket_types.hpp>

namespace boost {
namespace asio {
namespace ip {

/// Encapsulates the flags needed for TCP.
/**
 * The boost::asio::ip::tcp class contains flags necessary for TCP sockets.
 *
 * @par Thread Safety:
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Safe.
 *
 * @par Concepts:
 * Protocol.
 */
class tcp
{
public:
  /// The type of a TCP endpoint.
  typedef basic_endpoint<tcp> endpoint;

  /// The type of a resolver query.
  typedef basic_resolver_query<tcp> resolver_query;

  /// The type of a resolver iterator.
  typedef basic_resolver_iterator<tcp> resolver_iterator;

  /// Construct to represent the IPv4 TCP protocol.
  static tcp v4()
  {
    return tcp(PF_INET);
  }

  /// Construct to represent the IPv4 TCP protocol.
  static tcp v6()
  {
    return tcp(PF_INET6);
  }

  /// Obtain an identifier for the type of the protocol.
  int type() const
  {
    return SOCK_STREAM;
  }

  /// Obtain an identifier for the protocol.
  int protocol() const
  {
    return IPPROTO_TCP;
  }

  /// Obtain an identifier for the protocol family.
  int family() const
  {
    return family_;
  }

  /// The TCP socket type.
  typedef basic_stream_socket<tcp> socket;

  /// The TCP acceptor type.
  typedef basic_socket_acceptor<tcp> acceptor;

  /// The TCP resolver type.
  typedef basic_resolver<tcp> resolver;

  /// The TCP iostream type.
  typedef basic_socket_iostream<tcp> iostream;

  /// Socket option for disabling the Nagle algorithm.
  /**
   * Implements the IPPROTO_TCP/TCP_NODELAY socket option.
   *
   * @par Examples:
   * Setting the option:
   * @code
   * boost::asio::ipv6::tcp::socket socket(io_service); 
   * ...
   * boost::asio::ipv6::tcp::no_delay option(true);
   * socket.set_option(option);
   * @endcode
   *
   * @par
   * Getting the current option value:
   * @code
   * boost::asio::ipv6::tcp::socket socket(io_service); 
   * ...
   * boost::asio::ipv6::tcp::no_delay option;
   * socket.get_option(option);
   * bool is_set = option.get();
   * @endcode
   *
   * @par Concepts:
   * Socket_Option, Boolean_Socket_Option.
   */
#if defined(GENERATING_DOCUMENTATION)
  typedef implementation_defined no_delay;
#else
  typedef boost::asio::detail::socket_option::boolean<
    IPPROTO_TCP, TCP_NODELAY> no_delay;
#endif

private:
  // Construct with a specific family.
  explicit tcp(int family)
    : family_(family)
  {
  }

  int family_;
};

} // namespace ip
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_IP_TCP_HPP
