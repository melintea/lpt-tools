/*
 *  $Id: $
 *
 *  Copyright 2023 Aurelian Melinte. 
 *  Released under LGPL 3.0 or later. 
 *
 *  Check std::move damage with PAPI.
 * 
 *  Notes:
 *  * measurements on an 4-cpu Intel(R) Pentium(R) Gold G5420 CPU @ 3.80GHz
 *   - reading moved objects increase cycles by about 50-150 %
 *   - reading moved objects increase cache misses by 80-140 %
 */

#include <lpt/papi/papi.h>

#include <barrier>
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
#  warning Unknown hardware_constructive_interference_size, check values above
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
 * Fill a vector via a number of producer threads.
 */
constexpr const size_t numThreads = 50;
std::barrier startSync(numThreads);
std::mutex mtx;

void fill(strvec& vec, const char* data, size_t dataSize)
{
    auto worker = [&](size_t nLoops, strvec& vec, const char* data, size_t dataSize)
    {
        startSync.arrive_and_wait();
        for (auto i = 0; i < nLoops; ++i) {
            std::lock_guard<std::mutex> guard(mtx);
            vec.emplace_back(data, dataSize);
        }
    };

    std::vector<std::jthread> threads;

    for (auto i = 0; i < numThreads; ++i) {
        threads.emplace_back(worker, vecSize/numThreads, std::ref(vec), data, dataSize);
    }
}

//-----------------------------------------------------------------------------
void as_percent(const std::string&                tag,
                const counters::measurement_data& data,
                const counters::measurement_data& base)
{
    if ( ! tag.empty()) { std::cout << tag << ": "; };
    std::cout << "Percents of: "<< data.tag() << " based over " << base.tag() << "\n";
    for (auto i = 0; i < data.size(); ++i) {
        double dataPoint(data.values()[i]);
        double basePoint(base.values()[i]);
        auto percent = basePoint != 0
                     ? (((dataPoint - basePoint)/basePoint) * 100)
                     : 0 ;
        std::cout << counters::name(i) << ": " << percent << '\n';
    }
    std::cout << std::endl;
}

void as_percent(const counters::measurement_data& data,
                const counters::measurement_data& base)
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

   strvec  copyConstructed;
   strvec  moveConstructed;

   copyConstructed.reserve(vecSize);
   moveConstructed.reserve(vecSize);

   std::cout << "*\n"
                "* Hardware: \n"
                "*\n";
   lpt::papi::hardware().print(std::cout);

   counters ctrs;
   auto cout_measurement = [](const counters::measurement_data* measure) -> void {
                                  const auto& vals(measure->values());
                                  std::cout << measure->tag() << '\n';
                                  for (auto i = 0; i < measure->size(); ++i) {
                                      std::cout << counters::name(i) << ": " << vals[i] << '\n';
                                  }
                                  std::cout << std::endl;
                            };

   std::cout << "**\n"
                "** Over cache line size \n"
                "**\n";
   {
       copyConstructed.clear();
       moveConstructed.clear();

       counters::measurement_data copyConstruct;
       counters::measurement_data moveConstruct;
       counters::measurement_data copyConstructRead;
       counters::measurement_data moveConstructRead;

       {
           counters::measurement pc("Baseline copy >64 constructed",
                                    ctrs,
                                    cout_measurement);
           for (auto i = 0; i < vecSize; ++i)
           {
               copyConstructed.push_back(overCacheLineStr);
           }

           copyConstruct = pc.data();
       }
       {
           counters::measurement pc("Move >64 constructed",
                                    ctrs,
                                    cout_measurement);
           for (auto i = 0; i < vecSize; ++i)
           {
               moveConstructed.emplace_back(overCacheLine, overCacheLineSize);
           }

           moveConstruct = pc.data();
       }
       {
           counters::measurement pc("Baseline read copy >64 constructed",
                                    ctrs,
                                    cout_measurement);
           for (auto i = 0; i < vecSize; ++i)
           {
               const auto& str = copyConstructed[i];
               // Note: c should be volatile
               for (auto j = 0; j < str.size(); ++j) { char c = str[j]; }
           }

           copyConstructRead = pc.data();
       }
       {
           counters::measurement pc("Read move >64 constructed",
                                    ctrs,
                                    cout_measurement);
           for (auto i = 0; i < vecSize; ++i)
           {
               const auto& str = moveConstructed[i];
               for (auto j = 0; j < str.size(); ++j) { char c = str[j]; }
           }

           moveConstructRead = pc.data();
       }

       std::cout << "*\n"
                    "* Copy less move over cache line size \n"
                    "*\n";
        counters::measurement_data copyLessMoveConstruct(copyConstruct - moveConstruct);
        copyLessMoveConstruct._tag = "Construct: copy - move";
        cout_measurement(&copyLessMoveConstruct);
        as_percent(moveConstruct, copyConstruct);
        counters::measurement_data copyLessMoveConstructRead(copyConstructRead - moveConstructRead);
        copyLessMoveConstructRead._tag = "Read: copy - move";
        cout_measurement(&copyLessMoveConstructRead);
        as_percent(moveConstructRead, copyConstructRead);

        {
           fill(moveConstructed, overCacheLine, overCacheLineSize);

           counters::measurement pc("Read move >64 constructed",
                                    ctrs,
                                    cout_measurement);
           for (auto i = 0; i < vecSize; ++i)
           {
               const auto& str = moveConstructed[i];
               for (auto j = 0; j < str.size(); ++j) { char c = str[j]; }
           }

           moveConstructRead = pc.data();
        }
        counters::measurement_data copyLessMoveConstructRead2(copyConstructRead - moveConstructRead);
        copyLessMoveConstructRead2._tag = "Diffusion read: copy - move";
        cout_measurement(&copyLessMoveConstructRead2);
        as_percent("Diffusion", moveConstructRead, copyConstructRead);
   }

   std::cout << "**\n"
                "** Under cache line size \n"
                "**\n";
   {
       copyConstructed.clear();
       moveConstructed.clear();

       counters::measurement_data copyConstruct;
       counters::measurement_data moveConstruct;
       counters::measurement_data copyConstructRead;
       counters::measurement_data moveConstructRead;

       {
           counters::measurement pc("Baseline copy <64 constructed",
                                    ctrs,
                                    cout_measurement);
           for (auto i = 0; i < vecSize; ++i)
           {
               copyConstructed.push_back(underCacheLineStr);
           }

           copyConstruct = pc.data();
       }
       {
           counters::measurement pc("Move <64 constructed",
                                    ctrs,
                                    cout_measurement);
           for (auto i = 0; i < vecSize; ++i)
           {
               moveConstructed.emplace_back(underCacheLine, underCacheLineSize);
           }

           moveConstruct = pc.data();
       }
       {
           counters::measurement pc("Baseline read copy <64 constructed",
                                    ctrs,
                                    cout_measurement);
           for (auto i = 0; i < vecSize; ++i)
           {
               const auto& str = copyConstructed[i];
               for (auto j = 0; j < str.size(); ++j) { char c = str[j]; }
           }

           copyConstructRead = pc.data();
       }
       {
           counters::measurement pc("Read move <64 constructed",
                                    ctrs,
                                    cout_measurement);
           for (auto i = 0; i < vecSize; ++i)
           {
               const auto& str = moveConstructed[i];
               for (auto j = 0; j < str.size(); ++j) { char c = str[j]; }
           }

           moveConstructRead = pc.data();
       }

       std::cout << "*\n"
                    "* Copy less move under cache line size \n"
                    "*\n";
        counters::measurement_data copyLessMoveConstruct(copyConstruct - moveConstruct);
        copyLessMoveConstruct._tag = "Construct: copy - move";
        cout_measurement(&copyLessMoveConstruct);
        as_percent(moveConstruct, copyConstruct);
        counters::measurement_data copyLessMoveConstructRead(copyConstructRead - moveConstructRead);
        copyLessMoveConstructRead._tag = "Read: copy - move";
        cout_measurement(&copyLessMoveConstructRead);
        as_percent(moveConstructRead, copyConstructRead);

        {
           fill(moveConstructed, underCacheLine, underCacheLineSize);
           
           counters::measurement pc("Read move <64 constructed",
                                    ctrs,
                                    cout_measurement);
           for (auto i = 0; i < vecSize; ++i)
           {
               const auto& str = moveConstructed[i];
               for (auto j = 0; j < str.size(); ++j) { char c = str[j]; }
           }

           moveConstructRead = pc.data();
        }
        counters::measurement_data copyLessMoveConstructRead2(copyConstructRead - moveConstructRead);
        copyLessMoveConstructRead2._tag = "Diffusion read: copy - move";
        cout_measurement(&copyLessMoveConstructRead2);
        as_percent("Diffusion", moveConstructRead, copyConstructRead);
   }

   std::cout << "*\n"
                "* Accumulated data \n"
                "*\n";
    ctrs.print(std::cout);
    std::cout << std::endl;

   exit(EXIT_SUCCESS);    
}

