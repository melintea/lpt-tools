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
 *  * measurements on an 4-cpu Intel(R) Pentium(R) Gold G5420 CPU @ 3.80GHz
 *   - cycles: reading moved objects increase cycles by about 400+ %
 *   - cache misses: reading moved objects increase cache misses by 300+ %
 */

#include <lpt/papi/papi.hpp>

#include <atomic>
#include <barrier>
#include <chrono>
#include <ctime>
#include <mutex>
#include <new>
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
#endif

constexpr const size_t vecSize = 10'000'000;

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
void as_percent(const std::string&                tag,
                const counters::datapoint& data,
                const counters::datapoint& base)
{
    if ( ! tag.empty()) { std::cout << tag << ": "; };
    std::cout << "Percents of: "<< data.tag() << " based over " << base.tag() << "\n"
              << "Negative: data is smaller than base\n";
    counters::datapoint::percents pcts(data.as_percent_of(base));
    std::cout << pcts << std::endl;
}

void as_percent(const counters::datapoint& data,
                const counters::datapoint& base)
{
    return as_percent({}, data, base);
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
   counters::datapoint copyConstructRead;
   counters::datapoint moveConstructRead;

   copyConstructedData.reserve(vecSize);
   moveConstructedData.reserve(vecSize);

   std::cout << "*\n"
                "* Hardware: \n"
                "*\n";
   lpt::papi::hardware().print(std::cout);

   counters ctrs;
   auto cout_measurement = [](const counters::datapoint* measure) -> void {
                                  const auto& vals(measure->values());
                                  std::cout << measure->tag() << '\n';
                                  for (auto i = 0; i < measure->size(); ++i) {
                                      std::cout << counters::name(i) << ": " << vals[i] << '\n';
                                  }
                                  std::cout << std::endl;
                            };

   std::cout << "**\n"
                "** Under cache line size \n"
                "**\n";
   {
       strvec& data(copyConstructedData);
       counters::datapoint& measurement(copyConstructRead);

       // prefill
       data.clear();
       for (auto i = 0; i < vecSize; ++i)
       {
           data.push_back(underCacheLineStr);
       }

       // Churn memory for churnTime
       fill_copy(data, underCacheLineStr);

       {
           counters::measurement pc("Baseline read of copy <64 constructed",
                                    ctrs,
                                    cout_measurement);
           for (auto i = 0; i < vecSize; ++i)
           {
               const auto& str = data[i];
               for (auto j = 0; j < str.size(); ++j) { volatile char c = str[j]; }
           }

           measurement = pc.data();
       }
   }
   {
       strvec& data(moveConstructedData);
       counters::datapoint& measurement(moveConstructRead);

       data.clear();
       for (auto i = 0; i < vecSize; ++i)
       {
           data.push_back(underCacheLineStr);
       }

       // Churn memory for churnTime
       fill_move(data, overCacheLine, overCacheLineSize);

       {
           counters::measurement pc("Baseline read of move <64 constructed",
                                    ctrs,
                                    cout_measurement);
           for (auto i = 0; i < vecSize; ++i)
           {
               const auto& str = data[i];
               for (auto j = 0; j < str.size(); ++j) { volatile char c = str[j]; }
           }

           measurement = pc.data();
       }
   }

   counters::datapoint copyLessMoveConstructRead(copyConstructRead - moveConstructRead);
   copyLessMoveConstructRead._tag = "Diffusion read: copy - move (negative if move > copy)";
   cout_measurement(&copyLessMoveConstructRead);
   as_percent("Diffusion (positive: move is worse)", moveConstructRead, copyConstructRead);

   std::cout << std::endl;

   exit(EXIT_SUCCESS);    
}

