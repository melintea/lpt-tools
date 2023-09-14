/*
 *  $Id: $
 *
 *  Copyright 2023 Aurelian Melinte. 
 *  Released under LGPL 3.0 or later. 
 * 
 *  Lock-free performance versus locks.
 *
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

    std::atomic<unsigned long> finalSum{0};

    auto worker = [&](size_t nLoops)
    {
        startSync.arrive_and_wait();

        for (size_t i = 0; i < nLoops ; ++i) {
            finalSum += i;
        }
        
        stopSync.arrive_and_wait();
    };

    std::vector<std::jthread> threads;
    for (auto i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, numLoops);
    }
}

void mutexed(size_t numThreads, size_t numLoops)
{
    std::barrier startSync(numThreads);
    std::barrier stopSync(numThreads);

    std::mutex    mtx;
    unsigned long finalSum{0};

    auto worker = [&](size_t nLoops)
    {
        startSync.arrive_and_wait();

        unsigned long localSum{0};
        for (size_t i = 0; i < nLoops ; ++i) {
            localSum += i;
        }
        {
            std::lock_guard<std::mutex> lock(mtx);
            finalSum += localSum;
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
    constexpr const size_t numLoops = 1'000'000;

    for (size_t nThreads : {1, 2, 4, 8, 16, 32, 64, 128})
    {
        {
            std::string label = "lockfree " + std::to_string(nThreads);
            lpt::chrono::measurement tm(label,
                                        [](auto&& tag, auto&& dur) -> decltype(auto) {
                                            std::cout << std::setw(15) << tag << ": " << dur.count() << std::endl;
                                        });

            lockfree(nThreads, numLoops);
        }
        {
            std::string label = "mutex " + std::to_string(nThreads);
            lpt::chrono::measurement tm(label,
                                        [](auto&& tag, auto&& dur) -> decltype(auto) {
                                            std::cout << std::setw(15) << tag << ": " << dur.count() << std::endl;
                                        });

            mutexed(nThreads, numLoops);
        }
    }

    return EXIT_SUCCESS; 
}

