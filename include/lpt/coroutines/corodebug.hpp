/*
 *  $Id: $
 *
 *  Copyright 2025 Aurelian Melinte.
 *  Released under GPL 3.0 or later.
 *
 */

#ifndef INCLUDED_debug_hpp_540364bb_c1da_4b40_8b6a_9793b8589616
#define INCLUDED_debug_hpp_540364bb_c1da_4b40_8b6a_9793b8589616

#pragma once


#include <cassert>
#include <coroutine>
#include <exception>
#include <iostream>
#include <stacktrace>

/*
 * Coroutines debugging utilities.
 * @see original at:
 *    - https://godbolt.org/z/d7EPTGTdd
 *    - https://www.youtube.com/watch?v=lKUVuaUbRDk
 *
 * This works well under MSVC in conjunction with the natvis file.
 * Not so much with g++
 *
 * Compile with:
 *    g++ -fcoroutines -std=c++23 -O0 -ggdb -fdump-lang-coro // gcc 15.1
 */

namespace lpt {

// Overcome standard committee's ovesights
namespace hacks {

// @see https://github.com/dfrib/accessprivate/blob/master/include/accessprivate/accessprivate.h
template <auto MEM_PTR>
struct private_accessor {
    static constexpr auto _memPtr = MEM_PTR;
    struct delegate;
};

#define DEFINE_ACCESSOR(qualified_class_name, class_data_member)                    \
    template <>                                                                     \
    struct private_accessor<&qualified_class_name::class_data_member>::delegate {   \
        friend auto& get_##class_data_member(qualified_class_name& obj) {           \
            return obj.*_memPtr;                                                    \
        }                                                                           \
    };                                                                              \
    auto &get_##class_data_member(qualified_class_name& obj); 


#if defined(__GNUG__)
DEFINE_ACCESSOR(std::stacktrace_entry, _M_pc);
struct stack_frame : public std::stacktrace_entry
{
    stack_frame(void* addr) : std::stacktrace_entry()
    {
        hacks::get__M_pc(*this) = reinterpret_cast<std::stacktrace_entry::native_handle_type>(addr);
    }
    
}; // stack_frame
#elif defined(__clang__)
struct stack_frame : public std::stacktrace_entry
{
    stack_frame(void* addr) : std::stacktrace_entry() {}
}; // stack_frame
#elif defined(__MSVC_VER)
struct stack_frame : public std::stacktrace_entry
{
    stack_frame(void* addr) : std::stacktrace_entry() {}
}; // stack_frame
#else
struct stack_frame : public std::stacktrace_entry
{
    stack_frame(void* addr) : std::stacktrace_entry() {}
}; // stack_frame
#endif

// gdb+gcc helper. @see the g++ coroutine header
struct n4861_frame
{
    void (*_resume)()  = nullptr;
    void (*_destroy)() = nullptr;

    void print(std::ostream& os) const
    {
        void* resumePoint(reinterpret_cast<void*>(_resume));
        os        << "resume:  " << std::hex << resumePoint << ' '
                  << std::dec << stack_frame(resumePoint);
          
        //os << '\n'<< "destroy: " << std::hex << reinterpret_cast<void*>(_destroy);
    }
    
    friend std::ostream& operator<<(std::ostream& os, const n4861_frame* pf)
    {
        if (pf) {
            pf->print(os);
        }
        return os;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const n4861_frame& f)
    {
        return os;
    }
}; // n4861_frame

} // hacks


/*
 * Usage:
 *     struct promise_base : public lpt::coroframe<promise_base> {...}
 */
template <typename PROMISE_T>
struct coroframe
{
    PROMISE_T* _parent = nullptr;  // link to parent in the async call stack
    PROMISE_T* _child  = nullptr;  // link to child in the async call stack

    // Upon co_awaiting the task, populate the fields
    // to establish the links between parent and child
    // Call in:
    //     auto await_suspend(std::coroutine_handle<OTHER_PROMISE_T> h_other)
    template<typename OTHER_PROMISE_T>
    void on_await_suspend(std::coroutine_handle<OTHER_PROMISE_T> other) 
    {
        assert(_parent == nullptr);
        _parent = &(other.promise());

        assert(other.promise()._child == nullptr);
        other.promise()._child = static_cast<OTHER_PROMISE_T*>(this);
    }
    
    // Unregister ourselves from our parent when we resume
    // Call in:
    //    T await_resume()
    void on_await_resume() 
    {
        _parent->_child = nullptr;
    }
    
    const hacks::n4861_frame* parent_frame() const
    {
        if ( !_parent) return nullptr;
        return reinterpret_cast<hacks::n4861_frame*>(std::coroutine_handle<PROMISE_T>::from_promise(*_parent).address());
    }
    
    const hacks::n4861_frame* this_frame() const
    {
        return reinterpret_cast<hacks::n4861_frame*>(std::coroutine_handle<PROMISE_T>::from_promise(*this).address());
    }
    
    const hacks::n4861_frame* child_frame() const
    {
        if ( !_child) return nullptr;
        return reinterpret_cast<hacks::n4861_frame*>(std::coroutine_handle<PROMISE_T>::from_promise(*_child).address());
    }

    void print(std::ostream& os) const
    {
        if (_parent) {
            os << parent_frame();
            //os         << "parent: " << parent_frame();
        }
        //if (_child) {
        //    os << '\n' << "child:  " << child_frame();
        //}
    }
    
    friend std::ostream& operator<<(std::ostream& os, const coroframe<PROMISE_T>* p)
    {
        if (p) {
            p->print(os);
        } else {
            os << "(null coroframe)";
        }
        return os;
    }
    friend std::ostream& operator<<(std::ostream& os, const coroframe<PROMISE_T>& cs)
    {
        cs.print(os);
        return os;
    }
    
}; // coroframe

/* 
 * Helper awaitable for retrieving the coroframes of the current coroutine
 * Usage:
 *     Async<int>::promise_type* callstack = co_await lpt::corostack<Async<int>::promise_type>{};
 *     std::cout << lpt::corostack(callstack) << '\n';
 */
template <typename PROMISE_T>
struct corostack 
{
    PROMISE_T* _ret = nullptr;
    
    corostack(PROMISE_T* ret) : _ret(ret) {}
    corostack() = default;
    
    bool await_ready() { return false; }

    template<typename OTHER_PROMISE_T>
    auto await_suspend(std::coroutine_handle<OTHER_PROMISE_T> h) 
    {
        _ret = &(h.promise());
        return h;
    }
    
    PROMISE_T* await_resume() 
    {
        return _ret;
    }
    
    void print(std::ostream& os) const
    {
        os << "Async Stack:\n"; 
    
        PROMISE_T* pf(_ret);
        while (pf) {
            auto pff = reinterpret_cast<hacks::n4861_frame*>(std::coroutine_handle<PROMISE_T>::from_promise(*pf).address());
            os << pff << '\n';
            pf = static_cast<PROMISE_T*>(pf->_parent);
        }
    }
    
    friend std::ostream& operator<<(std::ostream& os, const corostack<PROMISE_T>* p)
    {
        if (p) {
            p->print(os);
        } else {
            os << "(null corostack)";
        }
        return os;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const corostack<PROMISE_T>& cs)
    {
        cs.print(os);
        return os;
    }
}; // corostack

} //namespace lpt


#endif //#define INCLUDED_debug_hpp_540364bb_c1da_4b40_8b6a_9793b8589616
