//
// posix_mutex.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_POSIX_MUTEX_HPP
#define BOOST_ASIO_DETAIL_POSIX_MUTEX_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/config.hpp>
#include <boost/asio/detail/pop_options.hpp>

#if defined(BOOST_HAS_PTHREADS)

#include <boost/asio/detail/push_options.hpp>
#include <boost/throw_exception.hpp>
#include <pthread.h>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/system_exception.hpp>
#include <boost/asio/detail/noncopyable.hpp>
#include <boost/asio/detail/scoped_lock.hpp>

namespace boost {
namespace asio {
namespace detail {

class posix_mutex
  : private noncopyable
{
public:
  typedef boost::asio::detail::scoped_lock<posix_mutex> scoped_lock;

  // Constructor.
  posix_mutex()
  {
    int error = ::pthread_mutex_init(&mutex_, 0);
    if (error != 0)
    {
      system_exception e("mutex", error);
      boost::throw_exception(e);
    }
  }

  // Destructor.
  ~posix_mutex()
  {
    ::pthread_mutex_destroy(&mutex_);
  }

  // Lock the mutex.
  void lock()
  {
    int error = ::pthread_mutex_lock(&mutex_);
    if (error != 0)
    {
      system_exception e("mutex", error);
      boost::throw_exception(e);
    }
  }

  // Unlock the mutex.
  void unlock()
  {
    int error = ::pthread_mutex_unlock(&mutex_);
    if (error != 0)
    {
      system_exception e("mutex", error);
      boost::throw_exception(e);
    }
  }

private:
  ::pthread_mutex_t mutex_;
};

} // namespace detail
} // namespace asio
} // namespace boost

#endif // defined(BOOST_HAS_PTHREADS)

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_POSIX_MUTEX_HPP
