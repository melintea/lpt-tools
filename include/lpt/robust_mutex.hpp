/*
 *  $Id: $
 *
 *  Copyright 2026 Aurelian Melinte.
 *  Released under GPL 3.0 or later.
 *
 */

#ifndef INCLUDED_robust_mutex_hpp_1ca9f675_0791_43b1_a013_d5fb2b5d45ef
#define INCLUDED_robust_mutex_hpp_1ca9f675_0791_43b1_a013_d5fb2b5d45ef

#pragma once

namespace lpt {

#include <atomic>   // for std::atomic
#include <mutex>    // for std::mutex
#include <thread>   // for std::thread::id



/**
 * Drop-in replacement for \c std::mutex with debug ownership validation
 * using pthread robust mutex API.
 *
 */
class robust_mutex // TODO no copy/move
{
public:

    robust_mutex(void)  = default;
    ~robust_mutex()     = default;

    robust_mutex( const robust_mutex& other )            = delete;
    robust_mutex& operator=( const robust_mutex& other ) = delete;

    robust_mutex( robust_mutex&& other )            = delete;
    robust_mutex& operator=( robust_mutex&& other ) = delete;

    void lock(void)
    {
        m_mutex.lock();
        m_ownerThread.store(std::this_thread::get_id(), std::memory_order_relaxed);
    }
    
    bool try_lock(void)
    {
        if(!m_mutex.try_lock())
        {
            return false;
        }

        m_ownerThread.store(std::this_thread::get_id(), std::memory_order_relaxed);
        return true;
    }
    
    void unlock(void)
    {
        m_ownerThread.store(std::thread::id{}, std::memory_order_relaxed);
        m_mutex.unlock();
    }
    
    bool is_locked_by_self(void) const
    {
        return m_ownerThread.load(std::memory_order_relaxed) == std::this_thread::get_id();
    }
    
private:

    std::mutex m_mutex; ///< TODO: pthread

    std::atomic<std::thread::id> m_ownerThread{}; ///< Thread that currently owns the mutex

}; // class robust_mutex


} //namespace lpt


#endif //#define INCLUDED_robust_mutex_hpp_1ca9f675_0791_43b1_a013_d5fb2b5d45ef
