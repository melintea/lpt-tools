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

#include <atomic>
#include <cerrno>
#include <thread>
#include <pthread.h>



/**
 * Drop-in replacement for \c std::mutex with debug ownership validation
 * using pthread robust mutex API.
 *
 */
class robust_mutex // TODO no copy/move
{
public:

    robust_mutex(void)
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setrobust(&attr,  PTHREAD_MUTEX_ROBUST);
        pthread_mutexattr_setpshared(&attr, PTHREAD_MPROCESS_SHARED);
    
        pthread_mutex_init(&_mutex, &attr);

        pthread_mutexattr_destroy(&attr);
    }
    
    ~robust_mutex()
    {
        pthread_mutex_destroy(&_mutex);
    }

    robust_mutex( const robust_mutex& other )            = delete;
    robust_mutex& operator=( const robust_mutex& other ) = delete;

    robust_mutex( robust_mutex&& other )            = delete;
    robust_mutex& operator=( robust_mutex&& other ) = delete;

    void lock(void)
    {
        int rc = pthread_mutex_lock(&_mutex);
        if (rc == EOWNERDEAD) {
            if (0 != pthread_mutex_consistent(&_mutex)) {
                perror(pthread_mutex_consistent);
            }
        }
        _ownerThread.store(std::this_thread::get_id(), std::memory_order_relaxed);
    }
    
    bool try_lock(void)
    {
        int rc = pthread_mutex_trylock(&_mutex);
        if (rc == EOWNERDEAD) {
            if (0 != pthread_mutex_consistent(&_mutex)) {
                perror(pthread_mutex_consistent);
            }
            // continue
        } else if (rc == EBUSY) {
            return false;
        } else {
            // TODO: diagnostic
            return false;
        }
    
        _ownerThread.store(std::this_thread::get_id(), std::memory_order_relaxed);
        return true;
    }
    
    void unlock(void)
    {
        _ownerThread.store(std::thread::id{}, std::memory_order_relaxed);
        pthread_mutex_unlock(&_mutex);
    }
    
    bool is_locked_by_self(void) const
    {
        return _ownerThread.load(std::memory_order_relaxed) == std::this_thread::get_id();
    }
    
private:

    pthread_mutext_t             _mutex; 
    std::atomic<std::thread::id> _ownerThread{}; ///< Thread that currently owns the mutex

}; // class robust_mutex


} //namespace lpt


#endif //#define INCLUDED_robust_mutex_hpp_1ca9f675_0791_43b1_a013_d5fb2b5d45ef
