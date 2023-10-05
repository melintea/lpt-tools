/*
 *  $Id: $
 *
 *  Copyright 2023 Aurelian Melinte. 
 *  Released under LGPL 3.0 or later. 
 * 
 *  Barrier perf tests
 *  g++ -I ../../../include -std=c++20 -O3 barrier1.cpp 
 */

#include <lpt/chrono.hpp>

#include <atomic>
#include <barrier>
#include <iostream>
#include <iomanip>
#include <mutex>
#include <thread>
#include <vector>

//-----------------------------------------------------------------------------
void lockfree(size_t numThreads, size_t numLoops)
{
    std::barrier startSync(numThreads);
    std::barrier stopSync(numThreads);

    // Obviously an incorrect end value but the test is about barrier costs
    static volatile long long finalSum{0};

    auto worker = [&](size_t nLoops)
    {
        startSync.arrive_and_wait();

        for (size_t i = 0; i < nLoops ; ++i) {
            finalSum = finalSum + 1;
        }
        
        stopSync.arrive_and_wait();
    };

    std::vector<std::jthread> threads;
    for (auto i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, numLoops);
    }
}

template <std::memory_order RBARRIER, std::memory_order WBARRIER>
void lockfree(size_t numThreads, size_t numLoops)
{
    std::barrier startSync(numThreads);
    std::barrier stopSync(numThreads);

    // Obviously an incorrect end value but the test is about barrier costs
    std::atomic<unsigned long long> finalSum{0}; 

    auto worker = [&](size_t nLoops)
    {
        startSync.arrive_and_wait();

        for (size_t i = 0; i < nLoops ; ++i) {
            finalSum.store(finalSum.load(RBARRIER) + 1, WBARRIER);
        }
        
        stopSync.arrive_and_wait();
    };

    std::vector<std::jthread> threads;
    for (auto i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, numLoops);
    }
}


//-----------------------------------------------------------------------------

int main()
{
    constexpr const size_t numLoops = 10'000'000;
    auto threadsList = {1, 2, 4, 8, 16, 32, 64, 128};

    for (size_t nThreads : threadsList)
    {
        {
            lpt::chrono::measurement tm("plain " + std::to_string(nThreads),
                                        [](auto&& tag, auto&& dur) -> decltype(auto) {
                                            std::cout << std::setw(15) << tag << ": " << dur.count() << std::endl;
                                        });
            lockfree(nThreads, numLoops);
        }
    }
    
    for (size_t nThreads : threadsList)
    {
        {
            lpt::chrono::measurement tm("r&w:cst " + std::to_string(nThreads),
                                        [](auto&& tag, auto&& dur) -> decltype(auto) {
                                            std::cout << std::setw(15) << tag << ": " << dur.count() << std::endl;
                                        });
            lockfree<std::memory_order_seq_cst, std::memory_order_seq_cst>(nThreads, numLoops);
        }
    }

    for (size_t nThreads : threadsList)
    {
        {
            lpt::chrono::measurement tm("r:acq/w:rel " + std::to_string(nThreads),
                                        [](auto&& tag, auto&& dur) -> decltype(auto) {
                                            std::cout << std::setw(15) << tag << ": " << dur.count() << std::endl;
                                        });
            lockfree<std::memory_order_acquire, std::memory_order_release>(nThreads, numLoops);
        }
    }

    for (size_t nThreads : threadsList)
    {
        {
            lpt::chrono::measurement tm("r:cons/w:rel " + std::to_string(nThreads),
                                        [](auto&& tag, auto&& dur) -> decltype(auto) {
                                            std::cout << std::setw(15) << tag << ": " << dur.count() << std::endl;
                                        });
            lockfree<std::memory_order_consume, std::memory_order_release>(nThreads, numLoops);
        }
    }

    for (size_t nThreads : threadsList)
    {
        {
            lpt::chrono::measurement tm("r&w:acqrel " + std::to_string(nThreads),
                                        [](auto&& tag, auto&& dur) -> decltype(auto) {
                                            std::cout << std::setw(15) << tag << ": " << dur.count() << std::endl;
                                        });
            lockfree<std::memory_order_acq_rel, std::memory_order_acq_rel>(nThreads, numLoops);
        }
    }

    for (size_t nThreads : threadsList)
    {
        {
            lpt::chrono::measurement tm("r&w:rlxd " + std::to_string(nThreads),
                                        [](auto&& tag, auto&& dur) -> decltype(auto) {
                                            std::cout << std::setw(15) << tag << ": " << dur.count() << std::endl;
                                        });
            lockfree<std::memory_order_relaxed, std::memory_order_relaxed>(nThreads, numLoops);
        }
    }

    return EXIT_SUCCESS; 
}

