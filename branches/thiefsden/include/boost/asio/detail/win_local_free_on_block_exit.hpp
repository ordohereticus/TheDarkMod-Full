//
// win_local_free_on_block_exit.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_WIN_LOCAL_FREE_ON_BLOCK_EXIT_HPP
#define BOOST_ASIO_DETAIL_WIN_LOCAL_FREE_ON_BLOCK_EXIT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/config.hpp>
#include <boost/asio/detail/pop_options.hpp>

#if defined(BOOST_WINDOWS) || defined(__CYGWIN__)

#include <boost/asio/detail/noncopyable.hpp>
#include <boost/asio/detail/socket_types.hpp>

namespace boost {
namespace asio {
namespace detail {

class win_local_free_on_block_exit
  : private noncopyable
{
public:
  // Constructor blocks all signals for the calling thread.
  explicit win_local_free_on_block_exit(void* p)
    : p_(p)
  {
  }

  // Destructor restores the previous signal mask.
  ~win_local_free_on_block_exit()
  {
    ::LocalFree(p_);
  }

private:
  void* p_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#endif // defined(BOOST_WINDOWS) || defined(__CYGWIN__)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_WIN_LOCAL_FREE_ON_BLOCK_EXIT_HPP
