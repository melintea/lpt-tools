/*
 *  $Id: $
 *
 *  Copyright 2023 Aurelian Melinte. 
 *  Released under LGPL 3.0 or later. 
 *
 */

#include <lpt/chrono_stats.hpp>
#include <lpt/compiler.hpp>
#include <lpt/papi/papi.hpp>

#include <atomic>
#include <barrier>
#include <chrono>
#include <ctime>
#include <mutex>
#include <new>
#include <ranges>
#include <string>
#include <thread>
#include <vector>

constexpr const size_t vecSize = 10'000'000;
constexpr const int    numLoops = 50;

using strvec = std::vector<std::string>;

//-----------------------------------------------------------------------------
/*
 * Fill-move a vector via a number of producer threads.
 */
constexpr const size_t numThreads = 50;
std::barrier startSync(numThreads);
std::barrier stopSync(numThreads);
std::atomic_bool stopFlag = false;
std::mutex mtx;

constexpr const auto churnTime = std::chrono::minutes(1);

void fill_copy(strvec& vec, const std::string& str)
{
    auto worker = [&](size_t nLoops, strvec& vec, const std::string& str)
    {
        startSync.arrive_and_wait();

        while ( ! stopFlag) {
            size_t i = std::rand() % vecSize;

            std::lock_guard<std::mutex> guard(mtx);
            vec[i] = str; // <== difference with fill_move
        }
        
        stopSync.arrive_and_wait();
    };

    std::srand(std::time(nullptr));
    stopFlag = false;

    std::vector<std::jthread> threads;
    for (auto i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, vecSize/numThreads, std::ref(vec), std::ref(str));
    }

    std::this_thread::sleep_for(churnTime);
    stopFlag = true;
}

void fill_move(strvec& vec, const char* data, size_t dataSize)
{
    auto worker = [&](size_t nLoops, strvec& vec, const char* data, size_t dataSize)
    {
        startSync.arrive_and_wait();

        while ( ! stopFlag) {
            std::string tmp(data, dataSize);
            size_t i = std::rand() % vecSize;

            std::lock_guard<std::mutex> guard(mtx);
            vec[i] = std::move(tmp); // <== difference with fill_copy
        }

        stopSync.arrive_and_wait();
    };

    std::srand(std::time(nullptr));
    stopFlag = false;

    std::vector<std::jthread> threads;
    for (auto i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, vecSize/numThreads, std::ref(vec), data, dataSize);
    }

    std::this_thread::sleep_for(churnTime);
    stopFlag = true;
}

//-----------------------------------------------------------------------------
int main()
{

   constexpr const char overCacheLine[] = 
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456"
       ;
    std::cout << "hardware_destructive_interference_size=" << lpt::papi::hardware::hardware_destructive_interference_size() << "\n";
    constexpr const size_t overCacheLineSize(sizeof(overCacheLine));
    static_assert(overCacheLineSize > lpt::papi::hardware::hardware_destructive_interference_size());
    const std::string overCacheLineStr(overCacheLine, overCacheLineSize);

    constexpr const char underCacheLine[] = 
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789"
       ;
    std::cout << "hardware_constructive_interference_size=" << lpt::papi::hardware::hardware_constructive_interference_size() << "\n";
    constexpr const size_t underCacheLineSize(sizeof(underCacheLine));
    const std::string underCacheLineStr(underCacheLine, underCacheLineSize);
    static_assert(underCacheLineSize <= lpt::papi::hardware::hardware_constructive_interference_size());

   strvec  copyConstructedData;
   strvec  moveConstructedData;

   copyConstructedData.reserve(vecSize);
   moveConstructedData.reserve(vecSize);

   lpt::chrono::dataset statsAsVals(lpt::chrono::timepoint::name());
   lpt::chrono::dataset statsAsPcts("% Move/Copy");

   for (auto loop : std::views::iota(1, numLoops+1))
   {
       std::cout << loop << " -------------------------------------\n";

       lpt::chrono::timepoint::duration_t copyConstructionDuration;
       lpt::chrono::timepoint::duration_t moveConstructionDuration;

        std::cout << "**\n"
                     "** Copy vs Move Constructed Under Cache Line Size \n"
                     "**\n";
        {
            strvec& data(copyConstructedData);

            // prefill
            data.clear();
            {
                lpt::chrono::measurement tm("Copy Construction Duration",
                                            [](auto&& tag, auto&& dur) -> decltype(auto) {});

                for (auto i = 0; i < vecSize; ++i)
                {
                    data.push_back(underCacheLineStr);
                }

                copyConstructionDuration = tm.elapsed();
            }

            //// Churn memory for churnTime
            //fill_copy(data, underCacheLineStr);

            {
                for (auto i = 0; i < vecSize; ++i)
                {
                    auto& str = data[i];
                    lpt::intel::disable_optimizer(str.data());
                    for (auto j = 0; j < str.size(); ++j) { volatile char c = str[j]; }
                }
                lpt::intel::barrier();
            }

            data.clear(); // Comment out for maximum cache thrashing
        }
        {
            strvec& data(moveConstructedData);

            // prefill
            data.clear();
            {
                lpt::chrono::measurement tm("Move Construction Duration",
                                            [](auto&& tag, auto&& dur) -> decltype(auto) {});

                for (auto i = 0; i < vecSize; ++i)
                {
                    data.emplace_back(underCacheLine, underCacheLineSize);
                }

                moveConstructionDuration = tm.elapsed();
                statsAsPcts(moveConstructionDuration, copyConstructionDuration);
                statsAsVals(moveConstructionDuration);
                std::cout << "Move gain (negative: move is faster) % : " << lpt::chrono::timepoint::as_percent_of(moveConstructionDuration, copyConstructionDuration) << "\n"
                          << "copyConstructionDuration: " << copyConstructionDuration.count() << "\n"
                          << "moveConstructionDuration: " << moveConstructionDuration.count() << "\n"
                          << "\n";
            }

            // Churn memory for churnTime
            fill_move(data, overCacheLine, overCacheLineSize);

            size_t  numReads(0); // when reads catch up the gained time
            {
                const auto     gain(copyConstructionDuration.count() - moveConstructionDuration.count());

                lpt::chrono::measurement tm("Read Moved Duration",
                                            [](auto&& tag, auto&& dur) -> decltype(auto) {});

                while (tm.elapsed().count() < gain)
                {
                    for (auto i = 0; i < vecSize; ++i)
                    {
                        auto& str = data[i];
                        lpt::intel::disable_optimizer(str.data());
                        for (auto j = 0; j < str.size(); ++j) { volatile char c = str[j]; }
                    }
                    lpt::intel::barrier();

                    ++numReads;
                    //std::cout << numReads << "\n";
                }
            }

            std::cout << "Gain equivalent to " << numReads << " reads \n";

            data.clear(); // Comment out for maximum cache thrashing
        }

        std::cout << std::endl;
    } // for tests' loop

    std::cout << "\n" << numLoops << " tests stats:\nPositive%: move is worse than copy\n"
              << statsAsVals
              << statsAsPcts
              << std::endl;

   exit(EXIT_SUCCESS);    
}

