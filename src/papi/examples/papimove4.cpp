/*
 *  $Id: $
 *
 *  Copyright 2023 Aurelian Melinte. 
 *  Released under LGPL 3.0 or later. 
 *
 *  Check std::move damage with PAPI.
 *  Churn a copy-contructed versus a move-constructed stucture for some time 
 *  then compare structure's read performance
 * 
 *  Notes:
 *  * sample measurement on an 4-cpu Intel(R) Pentium(R) Gold G5420 CPU @ 3.80GHz
 *    Move gain % : 51.4536
 *    copyConstructionDuration: 833027394 ns
 *    moveConstructionDuration: 404404514 ns
 *    Gain equivalent to 1 reads
 */

#include <lpt/chrono.hpp>
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

#ifdef __cpp_lib_hardware_interference_size
    using std::hardware_constructive_interference_size;
    using std::hardware_destructive_interference_size;
#else
    // 64 bytes on x86-64 │ L1_CACHE_BYTES │ L1_CACHE_SHIFT │ __cacheline_aligned │ ...
    constexpr std::size_t hardware_constructive_interference_size = 64;
    constexpr std::size_t hardware_destructive_interference_size  = 64;
#  warning Unknown hardware_constructive_interference_size, check values above
#endif

constexpr const size_t vecSize = 10'000'000;
constexpr const int    numLoops = 50;

using strvec = std::vector<std::string>;

   using counters = lpt::papi::counters<
         PAPI_TOT_INS // Total instructions"
       , PAPI_TOT_CYC // "Total cpu cycles"
       , PAPI_L1_DCM  // "L1 load  misses"
       // , PAPI_L1_STM  // "L1 store  missess"
       , PAPI_L2_DCM  // "L2 load  misses"
       //, PAPI_L3_DCM  // "L3 load  misses"
       // , PAPI_L2_STM  // "L2 store  missess"
       , PAPI_BR_MSP  // "Branch mispredictions"
   >;

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
       "123456"
       ;
    constexpr const size_t overCacheLineSize(sizeof(overCacheLine));
    static_assert(overCacheLineSize > hardware_destructive_interference_size);
    const std::string overCacheLineStr(overCacheLine, overCacheLineSize);

    constexpr const char underCacheLine[] = 
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789."
       "123456789"
       ;
    constexpr const size_t underCacheLineSize(sizeof(underCacheLine));
    const std::string underCacheLineStr(underCacheLine, underCacheLineSize);
    static_assert(underCacheLineSize <= hardware_constructive_interference_size);

   strvec  copyConstructedData;
   strvec  moveConstructedData;

   copyConstructedData.reserve(vecSize);
   moveConstructedData.reserve(vecSize);

   std::cout << "*\n"
                "* Hardware: \n"
                "*\n";
   lpt::papi::hardware().print(std::cout);

   //for (auto loop : std::views::iota(1, numLoops+1))
   {
       //std::cout << loop << " -------------------------------------\n";

       std::chrono::nanoseconds copyConstructionDuration;
       std::chrono::nanoseconds moveConstructionDuration;

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
            }

            std::cout << "Move gain % : " << (100 * (copyConstructionDuration.count() - moveConstructionDuration.count()) / (double)copyConstructionDuration.count()) << "\n"
                      << "copyConstructionDuration: " << copyConstructionDuration.count() << "\n"
                      << "moveConstructionDuration: " << moveConstructionDuration.count() << "\n"
                      << "\n";

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

   exit(EXIT_SUCCESS);    
}

