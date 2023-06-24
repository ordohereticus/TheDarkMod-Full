//////////////////////////////////////////////////////////////////////////////
//
// This file is the adaptation for Interprocess of boost/shared_ptr.hpp
//
// (C) Copyright Greg Colvin and Beman Dawes 1998, 1999.
// (C) Copyright Peter Dimov 2001, 2002, 2003
// (C) Copyright Ion Gaztanaga 2006-2008.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/interprocess for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_INTERPROCESS_SHARED_PTR_HPP_INCLUDED
#define BOOST_INTERPROCESS_SHARED_PTR_HPP_INCLUDED

#include <boost/interprocess/detail/config_begin.hpp>
#include <boost/interprocess/detail/workaround.hpp>

#include <boost/interprocess/detail/utilities.hpp>
#include <boost/interprocess/detail/cast_tags.hpp>
#include <boost/assert.hpp>
#include <boost/interprocess/smart_ptr/detail/shared_count.hpp>
#include <boost/interprocess/detail/mpl.hpp>
#include <boost/interprocess/detail/type_traits.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/smart_ptr/deleter.hpp>
#include <boost/static_assert.hpp>

#include <algorithm>            // for std::swap
#include <functional>           // for std::less
#include <typeinfo>             // for std::bad_cast
#include <iosfwd>               // for std::basic_ostream

//!\file
//!Describes the smart pointer shared_ptr

namespace boost{
namespace interprocess{

template<class T, class VoidAllocator, class Deleter> class weak_ptr;
template<class T, class VoidAllocator, class Deleter> class enable_shared_from_this;

namespace detail{

template<class T, class VoidAllocator, class Deleter>
inline void sp_enable_shared_from_this
  (shared_count<T, VoidAllocator, Deleter> const & pn,
   const typename pointer_to_other <typename shared_count<T, VoidAllocator, Deleter>::pointer, 
                                    enable_shared_from_this<T, VoidAllocator, Deleter> >::type &pe,
   const typename shared_count<T, VoidAllocator, Deleter>::pointer &px)
{
    if(pe != 0)
      pe->_internal_weak_this._internal_assign(pn);
}
/*
template<class T, class VoidAllocator, class Deleter>
inline void sp_enable_shared_from_this(shared_count<T, VoidAllocator, Deleter> const &, ...)
{}
*/
} // namespace detail

//!shared_ptr stores a pointer to a dynamically allocated object. 
//!The object pointed to is guaranteed to be deleted when the last shared_ptr pointing to 
//!it is destroyed or reset.
//!
//!shared_ptr is parameterized on 
//!T (the type of the object pointed to), VoidAllocator (the void allocator to be used
//!to allocate the auxiliary data) and Deleter (the deleter whose 
//!operator() will be used to delete the object.
//!
//!The internal pointer will be of the same pointer type as typename 
//!VoidAllocator::pointer type (that is, if typename VoidAllocator::pointer is 
//!offset_ptr<void>, the internal pointer will be offset_ptr<T>).
//!
//!Because the implementation uses reference counting, cycles of shared_ptr
//!instances will not be reclaimed. For example, if main() holds a
//!shared_ptr to A, which directly or indirectly holds a shared_ptr back
//!to A, A's use count will be 2. Destruction of the original shared_ptr
//!will leave A dangling with a use count of 1.
//!Use weak_ptr to "break cycles."
template<class T, class VoidAllocator, class Deleter>
class shared_ptr
{
   /// @cond
   private:
   typedef shared_ptr<T, VoidAllocator, Deleter> this_type;
   /// @endcond

   public:

   typedef T                                                   element_type;
   typedef T                                                   value_type;
   typedef typename detail::pointer_to_other
      <typename VoidAllocator::pointer, T>::type               pointer;
   typedef typename detail::add_reference
                     <value_type>::type                        reference;
   typedef typename detail::add_reference
                     <const value_type>::type                  const_reference;
   typedef typename detail::pointer_to_other
            <typename VoidAllocator::pointer, const Deleter>::type         const_deleter_pointer;
   typedef typename detail::pointer_to_other
            <typename VoidAllocator::pointer, const VoidAllocator>::type   const_allocator_pointer;

   public:
   //!Constructs an empty shared_ptr.
   //!Use_count() == 0 && get()== 0.
   shared_ptr()
      :  m_pn() // never throws
   {}

   //!Constructs a shared_ptr that owns the pointer p. Auxiliary data will be allocated
   //!with a copy of a and the object will be deleted with a copy of d.
   //!Requirements: Deleter and A's copy constructor must not throw.
   explicit shared_ptr(const pointer&p, const VoidAllocator &a = VoidAllocator(), const Deleter &d = Deleter())
      :  m_pn(p, a, d)
   {  
      //Check that the pointer passed is of the same type that
      //the pointer the allocator defines or it's a raw pointer
      typedef typename detail::pointer_to_other<pointer, T>::type ParameterPointer;
      BOOST_STATIC_ASSERT((detail::is_same<pointer, ParameterPointer>::value) ||
                          (detail::is_pointer<pointer>::value));
      //detail::sp_enable_shared_from_this( m_pn, p, p ); 
   }

   //!If r is empty, constructs an empty shared_ptr. Otherwise, constructs 
   //!a shared_ptr that shares ownership with r. Never throws.
   template<class Y>
   shared_ptr(shared_ptr<Y, VoidAllocator, Deleter> const & r)
      :  m_pn(r.m_pn) // never throws
   {}

   //!Constructs a shared_ptr that shares ownership with r and stores 
   //!a copy of the pointer stored in r.
   template<class Y>
   explicit shared_ptr(weak_ptr<Y, VoidAllocator, Deleter> const & r)
      :  m_pn(r.m_pn) // may throw
   {}

   /// @cond
   template<class Y>
   shared_ptr(shared_ptr<Y, VoidAllocator, Deleter> const & r, detail::static_cast_tag)
      :  m_pn( pointer(static_cast<T*>(detail::get_pointer(r.m_pn.get_pointer())))
             , r.m_pn) 
   {}

   template<class Y>
   shared_ptr(shared_ptr<Y, VoidAllocator, Deleter> const & r, detail::const_cast_tag)
      :  m_pn( pointer(const_cast<T*>(detail::get_pointer(r.m_pn.get_pointer())))
             , r.m_pn) 
   {}

   template<class Y>
   shared_ptr(shared_ptr<Y, VoidAllocator, Deleter> const & r, detail::dynamic_cast_tag)
      :  m_pn( pointer(dynamic_cast<T*>(detail::get_pointer(r.m_pn.get_pointer())))
             , r.m_pn) 
   {
      if(!m_pn.get_pointer()){ // need to allocate new counter -- the cast failed
         m_pn = detail::shared_count<T, VoidAllocator, Deleter>();
      }
   }
   /// @endcond

   //!Equivalent to shared_ptr(r).swap(*this).
   //!Never throws
   template<class Y>
   shared_ptr & operator=(shared_ptr<Y, VoidAllocator, Deleter> const & r)
   {
      m_pn = r.m_pn; // shared_count::op= doesn't throw
      return *this;
   }

   //!This is equivalent to:
   //!this_type().swap(*this);
   void reset()
   {  
      this_type().swap(*this);   
   }

   //!This is equivalent to:
   //!this_type(p, a, d).swap(*this);
   template<class Pointer>
   void reset(const Pointer &p, const VoidAllocator &a = VoidAllocator(), const Deleter &d = Deleter())
   {  
      //Check that the pointer passed is of the same type that
      //the pointer the allocator defines or it's a raw pointer
      typedef typename detail::pointer_to_other<Pointer, T>::type ParameterPointer;
      BOOST_STATIC_ASSERT((detail::is_same<pointer, ParameterPointer>::value) ||
                          (detail::is_pointer<Pointer>::value));
      this_type(p, a, d).swap(*this);  
   }

   //!Returns a reference to the
   //!pointed type
   reference operator* () const // never throws
   {  BOOST_ASSERT(m_pn.get_pointer() != 0);  return *m_pn.get_pointer(); }

   //!Returns the pointer pointing 
   //!to the owned object
   pointer operator-> () const // never throws
   {  BOOST_ASSERT(m_pn.get_pointer() != 0);  return m_pn.get_pointer();  }

   //!Returns the pointer pointing 
   //!to the owned object
   pointer get() const  // never throws
   {  return m_pn.get_pointer();  }

   /// @cond
   // implicit conversion to "bool"
   void unspecified_bool_type_func() const {}
   typedef void (this_type::*unspecified_bool_type)() const;

   operator unspecified_bool_type() const // never throws
   {  return !m_pn.get_pointer() ? 0 : &this_type::unspecified_bool_type_func;  }
   /// @endcond

   //!Not operator.
   //!Returns true if this->get() != 0, false otherwise
   bool operator! () const // never throws
   {  return !m_pn.get_pointer();   }

   //!Returns use_count() == 1.
   //!unique() might be faster than use_count()
   bool unique() const // never throws
   {  return m_pn.unique();  }

   //!Returns the number of shared_ptr objects, *this included,
   //!that share ownership with *this, or an unspecified nonnegative
   //!value when *this is empty.
   //!use_count() is not necessarily efficient. Use only for
   //!debugging and testing purposes, not for production code.
   long use_count() const // never throws
   {  return m_pn.use_count();  }

   //!Exchanges the contents of the two
   //!smart pointers.
   void swap(shared_ptr<T, VoidAllocator, Deleter> & other) // never throws
   {  m_pn.swap(other.m_pn);   }

   /// @cond

   template<class T2, class A2, class Deleter2> 
   bool _internal_less(shared_ptr<T2, A2, Deleter2> const & rhs) const
   {  return m_pn < rhs.m_pn;  }

   const_deleter_pointer get_deleter() const
   {  return m_pn.get_deleter(); }

//   const_allocator_pointer get_allocator() const
//   {  return m_pn.get_allocator(); }

   private:

   template<class T2, class A2, class Deleter2> friend class shared_ptr;
   template<class T2, class A2, class Deleter2> friend class weak_ptr;

   detail::shared_count<T, VoidAllocator, Deleter>   m_pn;    // reference counter
   /// @endcond
};  // shared_ptr

template<class T, class VoidAllocator, class Deleter, class U, class VoidAllocator2, class Deleter2> inline 
bool operator==(shared_ptr<T, VoidAllocator, Deleter> const & a, shared_ptr<U, VoidAllocator2, Deleter2> const & b)
{  return a.get() == b.get(); }

template<class T, class VoidAllocator, class Deleter, class U, class VoidAllocator2, class Deleter2> inline 
bool operator!=(shared_ptr<T, VoidAllocator, Deleter> const & a, shared_ptr<U, VoidAllocator2, Deleter2> const & b)
{  return a.get() != b.get(); }

template<class T, class VoidAllocator, class Deleter, class U, class VoidAllocator2, class Deleter2> inline 
bool operator<(shared_ptr<T, VoidAllocator, Deleter> const & a, shared_ptr<U, VoidAllocator2, Deleter2> const & b)
{  return a._internal_less(b);   }

template<class T, class VoidAllocator, class Deleter> inline 
void swap(shared_ptr<T, VoidAllocator, Deleter> & a, shared_ptr<T, VoidAllocator, Deleter> & b)
{  a.swap(b);  }

template<class T, class VoidAllocator, class Deleter, class U> inline
shared_ptr<T, VoidAllocator, Deleter> static_pointer_cast(shared_ptr<U, VoidAllocator, Deleter> const & r)
{  return shared_ptr<T, VoidAllocator, Deleter>(r, detail::static_cast_tag());   }

template<class T, class VoidAllocator, class Deleter, class U> inline 
shared_ptr<T, VoidAllocator, Deleter> const_pointer_cast(shared_ptr<U, VoidAllocator, Deleter> const & r)
{  return shared_ptr<T, VoidAllocator, Deleter>(r, detail::const_cast_tag()); }

template<class T, class VoidAllocator, class Deleter, class U> inline 
shared_ptr<T, VoidAllocator, Deleter> dynamic_pointer_cast(shared_ptr<U, VoidAllocator, Deleter> const & r)
{  return shared_ptr<T, VoidAllocator, Deleter>(r, detail::dynamic_cast_tag());  }

// get_pointer() enables boost::mem_fn to recognize shared_ptr
template<class T, class VoidAllocator, class Deleter> inline
T * get_pointer(shared_ptr<T, VoidAllocator, Deleter> const & p)
{  return p.get();   }

// operator<<
template<class E, class T, class Y, class VoidAllocator, class Deleter> inline
std::basic_ostream<E, T> & operator<< 
   (std::basic_ostream<E, T> & os, shared_ptr<Y, VoidAllocator, Deleter> const & p)
{  os << p.get();   return os;   }

//!Returns the type of a shared pointer
//!of type T with the allocator boost::interprocess::allocator allocator
//!and boost::interprocess::deleter deleter
//!that can be constructed in the given managed segment type.
template<class T, class ManagedMemory>
struct managed_shared_ptr
{
   typedef typename ManagedMemory::template allocator<void>::type void_allocator;
   typedef typename ManagedMemory::template deleter<T>::type      deleter;
   typedef shared_ptr< T, void_allocator, deleter>                type;
};

//!Returns an instance of a shared pointer constructed
//!with the default allocator and deleter from a pointer
//!of type T that has been allocated in the passed managed segment
template<class T, class ManagedMemory>
inline typename managed_shared_ptr<T, ManagedMemory>::type
   make_managed_shared_ptr(T *constructed_object, ManagedMemory &managed_memory)
{
   return typename managed_shared_ptr<T, ManagedMemory>::type
   ( constructed_object
   , managed_memory.template get_allocator<void>()
   , managed_memory.template get_deleter<T>()
   );
}


/*
// get_deleter (experimental)
template<class T, class VoidAllocator, class Deleter> 
typename detail::pointer_to_other<shared_ptr<T, VoidAllocator, Deleter>, Deleter>::type
   get_deleter(shared_ptr<T, VoidAllocator, Deleter> const & p)
{  return static_cast<Deleter *>(p._internal_get_deleter(typeid(Deleter)));   }
*/
} // namespace interprocess

/// @cond

#if defined(_MSC_VER) && (_MSC_VER < 1400)
// get_pointer() enables boost::mem_fn to recognize shared_ptr
template<class T, class VoidAllocator, class Deleter> inline
T * get_pointer(boost::interprocess::shared_ptr<T, VoidAllocator, Deleter> const & p)
{  return p.get();   }
#endif

/// @endcond

} // namespace boost

#include <boost/interprocess/detail/config_end.hpp>

#endif  // #ifndef BOOST_INTERPROCESS_SHARED_PTR_HPP_INCLUDED
