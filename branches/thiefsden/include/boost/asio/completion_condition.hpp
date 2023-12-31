//
// completion_condition.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_COMPLETION_CONDITION_HPP
#define BOOST_ASIO_COMPLETION_CONDITION_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <cstddef>
#include <boost/config.hpp>
#include <boost/asio/detail/pop_options.hpp>

namespace boost {
namespace asio {

namespace detail {

class transfer_all_t
{
public:
  typedef bool result_type;

  template <typename Error>
  bool operator()(const Error& err, std::size_t)
  {
    return !!err;
  }
};

class transfer_at_least_t
{
public:
  typedef bool result_type;

  explicit transfer_at_least_t(std::size_t minimum)
    : minimum_(minimum)
  {
  }

  template <typename Error>
  bool operator()(const Error& err, std::size_t bytes_transferred)
  {
    return !!err || bytes_transferred >= minimum_;
  }

private:
  std::size_t minimum_;
};

} // namespace detail

/**
 * @defgroup completion_condition Completion Condition Function Objects
 *
 * Function objects used for determining when a read or write operation should
 * complete.
 */
/*@{*/

/// Return a completion condition function object that indicates that a read or
/// write operation should continue until all of the data has been transferred,
/// or until an error occurs.
#if defined(GENERATING_DOCUMENTATION)
unspecified transfer_all();
#else
inline detail::transfer_all_t transfer_all()
{
  return detail::transfer_all_t();
}
#endif

/// Return a completion condition function object that indicates that a read or
/// write operation should continue until a minimum number of bytes has been
/// transferred, or until an error occurs.
#if defined(GENERATING_DOCUMENTATION)
unspecified transfer_at_least(std::size_t minimum);
#else
inline detail::transfer_at_least_t transfer_at_least(std::size_t minimum)
{
  return detail::transfer_at_least_t(minimum);
}
#endif

/*@}*/

} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_COMPLETION_CONDITION_HPP
