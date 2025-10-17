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
        other.promise()._child = static_cast<PROMISE_T*>(this);
    }
    
    // Unregister ourselves from our parent when we resume
    // Call in:
    //    T await_resume()
    void on_await_resume() 
    {
        _parent->_child = nullptr;
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
	    os << pf << '\n';
	    pf = static_cast<PROMISE_T*>(pf->_parent);
	}
    }
    
    friend std::ostream& operator<<(std::ostream& os, const corostack<PROMISE_T>& cs)
    {
        cs.print(os);
        return os;
    }
}; // corostack

} //namespace lpt


#endif //#define INCLUDED_debug_hpp_540364bb_c1da_4b40_8b6a_9793b8589616
