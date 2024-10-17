/*
 *  $Id: $
 *
 *  Copyright 2024 Aurelian Melinte.
 *  Released under GPL 3.0 or later.
 *
 *  You need a C++0x compiler.
 *
 */

#ifndef INCLUDED_shared_mutex_hpp_447ff94b_2ce6_4d22_8a20_e5678f4adbc8
#define INCLUDED_shared_mutex_hpp_447ff94b_2ce6_4d22_8a20_e5678f4adbc8

#pragma once

#include <relacy/relacy.hpp>

namespace std {

/*
 * Minimal implementation. Not fully standard compliant.
 * If SIGSEGV because of a null model* ModelChecker: this class cannot be used for
 * global vars.
 * Might hog the machine because of the while loops in the implementation.
 */
class shared_mutex 
{

    // refcount is 0 if no thread holds the lock.  
    // If > 0, the value represents the number of readers that have access.
    // If -1, a single writer has access.
    static constexpr int LOCK_WRITER = -1;
    static constexpr int LOCK_FREE   = 0;
    rl::atomic<int> _refcount{LOCK_FREE}; // model bug: initialization not seen

public:

    shared_mutex()  { 
        _refcount($).store(LOCK_FREE, rl::mo_release); 
    } 
    ~shared_mutex() = default;

    //shared_mutex( const shared_mutex& other )            = delete; //relacy: delete == RL_PROXY_DELETE for memmory ops
    //shared_mutex& operator=( const shared_mutex& other ) = delete;

    //shared_mutex( shared_mutex&& other )                 = delete;
    //shared_mutex& operator=( shared_mutex&& other )      = delete;
    
    void lock() // write 
    {
        int val{0};
        do {
            //thrd_yield();
            val = 0; // Can only take a write lock when refcount == 0
        } while ( ! _refcount($).compare_exchange_weak(val, LOCK_WRITER, rl::mo_acquire));
        RL_ASSERT(val == 0);
        RL_ASSERT(_refcount($) == LOCK_WRITER);
    }

    void unlock() // write 
    {
        _refcount($).store(LOCK_FREE, rl::mo_release);
        RL_ASSERT(_refcount($) >= LOCK_WRITER);
    }

    void lock_shared() // read 
    {
        int val{0};
        do {
            //thrd_yield(); 
            
            do {
                //thrd_yield();
                val = _refcount($).load(rl::mo_relaxed);
            } while (val == LOCK_WRITER); // spinning until the write lock is released
            RL_ASSERT(val == 0);

        } while ( ! _refcount($).compare_exchange_weak(val, val+1, rl::mo_acquire));
        RL_ASSERT(val >= 0);
        RL_ASSERT(_refcount($) > 0);
    }

    void unlock_shared() // read 
    {
        _refcount($).fetch_sub(1, rl::mo_release);
        RL_ASSERT(_refcount($) >= LOCK_WRITER);
    }
};


/*
 * Minimal implementation. Not fully standard compliant.
 */
template <typename MUTEX_T>
class unique_lock
{
public:

    explicit unique_lock(MUTEX_T& mtx) : _mutex(mtx) { _mutex.lock(); }
    //unique_lock()                              = delete;
    ~unique_lock() { _mutex.unlock(); }

    //unique_lock(const unique_lock&)            = delete;
    //unique_lock& operator=(const unique_lock&) = delete;
    //unique_lock(unique_lock&&)                 = delete;
    //unique_lock& operator=(unique_lock&&)      = delete;

private:

    MUTEX_T& _mutex;
}; //unique_lock


/*
 * Minimal implementation. Not fully standard compliant.
 */
template <typename MUTEX_T>
class shared_lock
{
public:

    explicit shared_lock(MUTEX_T& mtx) : _mutex(mtx) { _mutex.lock_shared(); }
    //shared_lock()                              = delete;
    ~shared_lock() { _mutex.unlock_shared(); }

    //shared_lock(const shared_lock&)            = delete;
    //shared_lock& operator=(const shared_lock&) = delete;
    //shared_lock(shared_lock&&)                 = delete;
    //shared_lock& operator=(shared_lock&&)      = delete;

private:

    MUTEX_T& _mutex;
}; //shared_lock

} //namespace std


#endif //#define INCLUDED_shared_mutex_hpp_447ff94b_2ce6_4d22_8a20_e5678f4adbc8
