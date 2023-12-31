//
// signal_blocker.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_DETAIL_SIGNAL_BLOCKER_HPP
#define BOOST_ASIO_DETAIL_SIGNAL_BLOCKER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <boost/config.hpp>
#include <boost/asio/detail/pop_options.hpp>

#if !defined(BOOST_HAS_THREADS)
# include <boost/asio/detail/null_signal_blocker.hpp>
#elif defined(BOOST_WINDOWS) || defined(__CYGWIN__)
# include <boost/asio/detail/win_signal_blocker.hpp>
#elif defined(BOOST_HAS_PTHREADS)
# include <boost/asio/detail/posix_signal_blocker.hpp>
#else
# error Only Windows and POSIX are supported!
#endif

namespace boost {
namespace asio {
namespace detail {

#if !defined(BOOST_HAS_THREADS)
typedef null_signal_blocker signal_blocker;
#elif defined(BOOST_WINDOWS) || defined(__CYGWIN__)
typedef win_signal_blocker signal_blocker;
#elif defined(BOOST_HAS_PTHREADS)
typedef posix_signal_blocker signal_blocker;
#endif

} // namespace detail
} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_DETAIL_SIGNAL_BLOCKER_HPP
