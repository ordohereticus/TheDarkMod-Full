//
// basic_deadline_timer.hpp
// ~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_ASIO_BASIC_DEADLINE_TIMER_HPP
#define BOOST_ASIO_BASIC_DEADLINE_TIMER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <boost/asio/detail/push_options.hpp>

#include <boost/asio/detail/push_options.hpp>
#include <cstddef>
#include <boost/config.hpp>
#include <boost/asio/detail/pop_options.hpp>

#include <boost/asio/basic_io_object.hpp>
#include <boost/asio/deadline_timer_service.hpp>
#include <boost/asio/error.hpp>

namespace boost {
namespace asio {

/// Provides waitable timer functionality.
/**
 * The basic_deadline_timer class template provides the ability to perform a
 * blocking or asynchronous wait for a timer to expire.
 *
 * Most applications will use the boost::asio::deadline_timer typedef.
 *
 * @par Thread Safety:
 * @e Distinct @e objects: Safe.@n
 * @e Shared @e objects: Unsafe.
 *
 * @par Concepts:
 * Async_Object, Error_Source.
 *
 * @sa @ref deadline_timer_reset
 *
 * @par Examples:
 * Performing a blocking wait:
 * @code
 * // Construct a timer without setting an expiry time.
 * boost::asio::deadline_timer timer(io_service);
 *
 * // Set an expiry time relative to now.
 * timer.expires_from_now(boost::posix_time::seconds(5));
 *
 * // Wait for the timer to expire.
 * timer.wait();
 * @endcode
 *
 * @par 
 * Performing an asynchronous wait:
 * @code
 * void handler(const boost::asio::error& error)
 * {
 *   if (!error)
 *   {
 *     // Timer expired.
 *   }
 * }
 *
 * ...
 *
 * // Construct a timer with an absolute expiry time.
 * boost::asio::deadline_timer timer(io_service,
 *     boost::posix_time::time_from_string("2005-12-07 23:59:59.000"));
 *
 * // Start an asynchronous wait.
 * timer.async_wait(handler);
 * @endcode
 */
template <typename Time_Type,
    typename Time_Traits = boost::asio::time_traits<Time_Type>,
    typename Service = deadline_timer_service<Time_Type, Time_Traits> >
class basic_deadline_timer
  : public basic_io_object<Service>
{
public:
  /// The type used for reporting errors.
  typedef boost::asio::error error_type;

  /// The time traits type.
  typedef Time_Traits traits_type;

  /// The time type.
  typedef typename traits_type::time_type time_type;

  /// The duration type.
  typedef typename traits_type::duration_type duration_type;

  /// Constructor.
  /**
   * This constructor creates a timer without setting an expiry time. The
   * expires_at() or expires_from_now() functions must be called to set an
   * expiry time before the timer can be waited on.
   *
   * @param io_service The io_service object that the timer will use to dispatch
   * handlers for any asynchronous operations performed on the timer.
   */
  explicit basic_deadline_timer(boost::asio::io_service& io_service)
    : basic_io_object<Service>(io_service)
  {
  }

  /// Constructor to set a particular expiry time as an absolute time.
  /**
   * This constructor creates a timer and sets the expiry time.
   *
   * @param io_service The io_service object that the timer will use to dispatch
   * handlers for any asynchronous operations performed on the timer.
   *
   * @param expiry_time The expiry time to be used for the timer, expressed
   * as an absolute time.
   */
  basic_deadline_timer(boost::asio::io_service& io_service,
      const time_type& expiry_time)
    : basic_io_object<Service>(io_service)
  {
    this->service.expires_at(this->implementation, expiry_time);
  }

  /// Constructor to set a particular expiry time relative to now.
  /**
   * This constructor creates a timer and sets the expiry time.
   *
   * @param io_service The io_service object that the timer will use to dispatch
   * handlers for any asynchronous operations performed on the timer.
   *
   * @param expiry_time The expiry time to be used for the timer, relative to
   * now.
   */
  basic_deadline_timer(boost::asio::io_service& io_service,
      const duration_type& expiry_time)
    : basic_io_object<Service>(io_service)
  {
    this->service.expires_from_now(this->implementation, expiry_time);
  }

  /// Cancel any asynchronous operations that are waiting on the timer.
  /**
   * This function forces the completion of any pending asynchronous wait
   * operations against the timer. The handler for each cancelled operation will
   * be invoked with the boost::asio::error::operation_aborted error code.
   *
   * Cancelling the timer does not change the expiry time.
   *
   * @return The number of asynchronous operations that were cancelled.
   */
  std::size_t cancel()
  {
    return this->service.cancel(this->implementation);
  }

  /// Get the timer's expiry time as an absolute time.
  /**
   * This function may be used to obtain the timer's current expiry time.
   * Whether the timer has expired or not does not affect this value.
   */
  time_type expires_at() const
  {
    return this->service.expires_at(this->implementation);
  }

  /// Set the timer's expiry time as an absolute time.
  /**
   * This function sets the expiry time. Any pending asynchronous wait
   * operations will be cancelled. The handler for each cancelled operation will
   * be invoked with the boost::asio::error::operation_aborted error code.
   *
   * See @ref deadline_timer_reset for more information on altering the expiry
   * time of an active timer.
   *
   * @param expiry_time The expiry time to be used for the timer.
   *
   * @return The number of asynchronous operations that were cancelled.
   */
  std::size_t expires_at(const time_type& expiry_time)
  {
    return this->service.expires_at(this->implementation, expiry_time);
  }

  /// Get the timer's expiry time relative to now.
  /**
   * This function may be used to obtain the timer's current expiry time.
   * Whether the timer has expired or not does not affect this value.
   */
  duration_type expires_from_now() const
  {
    return this->service.expires_from_now(this->implementation);
  }

  /// Set the timer's expiry time relative to now.
  /**
   * This function sets the expiry time. Any pending asynchronous wait
   * operations will be cancelled. The handler for each cancelled operation will
   * be invoked with the boost::asio::error::operation_aborted error code.
   *
   * See @ref deadline_timer_reset for more information on altering the expiry
   * time of an active timer.
   *
   * @param expiry_time The expiry time to be used for the timer.
   *
   * @return The number of asynchronous operations that were cancelled.
   */
  std::size_t expires_from_now(const duration_type& expiry_time)
  {
    return this->service.expires_from_now(this->implementation, expiry_time);
  }

  /// Perform a blocking wait on the timer.
  /**
   * This function is used to wait for the timer to expire. This function
   * blocks and does not return until the timer has expired.
   *
   * @throws boost::asio::error Thrown on failure.
   */
  void wait()
  {
    this->service.wait(this->implementation);
  }

  /// Start an asynchronous wait on the timer.
  /**
   * This function may be used to initiate an asynchronous wait against the
   * timer. It always returns immediately.
   *
   * For each call to async_wait(), the supplied handler will be called exactly
   * once. The handler will be called when:
   *
   * @li The timer has expired.
   *
   * @li The timer was cancelled, in which case the handler is passed the error
   * code boost::asio::error::operation_aborted.
   *
   * @param handler The handler to be called when the timer expires. Copies
   * will be made of the handler as required. The function signature of the
   * handler must be:
   * @code void handler(
   *   const boost::asio::error& error // Result of operation
   * ); @endcode
   * Regardless of whether the asynchronous operation completes immediately or
   * not, the handler will not be invoked from within this function. Invocation
   * of the handler will be performed in a manner equivalent to using
   * boost::asio::io_service::post().
   */
  template <typename Handler>
  void async_wait(Handler handler)
  {
    this->service.async_wait(this->implementation, handler);
  }
};

/**
 * @page deadline_timer_reset Changing an active deadline_timer's expiry time
 *
 * Changing the expiry time of a timer while there are pending asynchronous
 * waits causes those wait operations to be cancelled. To ensure that the action
 * associated with the timer is performed only once, use something like this:
 * used:
 *
 * @code
 * void on_some_event()
 * {
 *   if (my_timer.expires_from_now(seconds(5)) > 0)
 *   {
 *     // We managed to cancel the timer. Start new asynchronous wait.
 *     my_timer.async_wait(on_timeout);
 *   }
 *   else
 *   {
 *     // Too late, timer has already expired!
 *   }
 * }
 *
 * void on_timeout(const boost::asio::error& e)
 * {
 *   if (e != boost::asio::error::operation_aborted)
 *   {
 *     // Timer was not cancelled, take necessary action.
 *   }
 * }
 * @endcode
 *
 * @li The boost::asio::basic_deadline_timer::expires_from_now() function
 * cancels any pending asynchronous waits, and returns the number of
 * asynchronous waits that were cancelled. If it returns 0 then you were too
 * late and the wait handler has already been executed, or will soon be
 * executed. If it returns 1 then the wait handler was successfully cancelled.
 *
 * @li If a wait handler is cancelled, the boost::asio::error passed to it
 * contains the value boost::asio::error::operation_aborted.
 *
 * @sa boost::asio::basic_deadline_timer
 */

} // namespace asio
} // namespace boost

#include <boost/asio/detail/pop_options.hpp>

#endif // BOOST_ASIO_BASIC_DEADLINE_TIMER_HPP
