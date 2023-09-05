/*
 *  $Id: mtrace.cpp,v 1.3 2011/09/05 21:27:18 amelinte Exp amelinte $
 *
 *  mtrace()/muntrace() & tools.  Based on glibc 2.6.1. 
 */

#include <malloc.h>
#include <mcheck.h>

#include <cassert>

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include <limits>
#include <map>
#include <string>
#include <unordered_map>
#include <list>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>


#include <lpt/gnu_atomic_guard.hpp>
#include <lpt/call_stack_stats.hpp>
#include <lpt/mem_stats.hpp>
#include <lpt/timing.hpp>

#include "report.h" 
#include "config.h"




/*
 * Used for reporting
 */
typedef struct _pdelta {
    memsize_type num_bytes;       // Current total allocated on this stack
    long num_allocs;
    memsize_type delta;           // Bytes since previous delta
    lpt::stack::key_type key;

} pdelta;


typedef struct _reporting_stats {
    memsize_type max_num_allocations; 
    memsize_type max_num_stacks; 
    memsize_type total_bytes;
	
	_reporting_stats()
	    : max_num_allocations(0)
		, max_num_stacks(0)
		, total_bytes(0)
	{}
} reporting_stats;


typedef struct _stack_mem_data {
    memsize_type num_bytes;       // Current total allocated on this stack
    long num_allocs;
    memsize_type prev_num_bytes;  // Total at the previous delta

    _stack_mem_data()
        : num_bytes(0)
        , num_allocs(1)
        , prev_num_bytes(0)
    {}
} stack_mem_data;

typedef lpt::stack::call_stack<40>          stack_type;
typedef lpt::stack::extended_symbol_info    frame_info_type;
typedef lpt::stack::crc32key<stack_type >   key_algo_type;
typedef lpt::stack::call_stack_info< stack_type
                                   , frame_info_type
                                   >                                  call_stack_info_type;
typedef lpt::stack::stack_stats< stack_mem_data
                               , stack_type
                               , key_algo_type >                      stack_mem_stats_type;
typedef lpt::stack::stats_map<stack_mem_stats_type>::type             stack_mem_stats_map_type;

typedef struct _reporting_data {
    lpt::allocation_map_type    _alloc_map;
    stack_mem_stats_map_type    _stack_map;
    reporting_stats             _stats;
	
    lpt::allocation_map_type&    allocations() { return _alloc_map; }
    stack_mem_stats_map_type&    stacks()      { return _stack_map; }
    reporting_stats&             stats()       { return _stats;     }
} reporting_data;



typedef std::pair<lpt::stack::address_type, memsize_type>  mem_per_frame_type;
typedef std::map<lpt::stack::address_type, memsize_type>   mem_per_frame_map_type;


static reporting_data _reporting_data;




void
error(__ptr_t ptr, const char *msg, const char *file, int line)
{
}


static inline void 
serror(__ptr_t ptr, const char *msg, const char *file, int line)
{
//    lpt::stack::libc::basic_dump(msg);
//    ::error(ptr, msg, file, line);
}

//------------------------------------------------------------------------------
namespace libmemleak { 

static std::mutex _reporting_lock;

/* 
 * Stop tracing allocations once we enter our hooks (infinite loop). 
 */
static volatile __thread bool _in_trace = false;

#define LMAX(a, b)  ((a<b)?b:a)


void init()
{
    // Ensure the sigletons are instantiated before atexit()
    // They will be used atexit by muntrace().
    // Any valid address is good.
    frame_info_type(std::addressof(_reporting_lock));
}

void fini()
{
}


void alloc(__ptr_t ptr, memsize_type size)
{
    lpt::gnu_atomic_guard<volatile bool> let_me_in(&_in_trace, false, true);
    if (let_me_in.acquired())
    {
        stack_mem_stats_type stack(true);
        key_algo_type        keyer;
        lpt::stack::key_type key = keyer(stack);

        {//lock
            std::unique_lock<std::mutex> lock(_reporting_lock);

            stack_mem_stats_map_type::iterator itStack = _reporting_data.stacks().find(key);
            if (itStack == _reporting_data.stacks().end()) {
                _reporting_data.stacks()[key] = std::move(stack);
            }
            else {
                itStack->second.num_allocs++;
                itStack->second.num_bytes += size;
            }

            lpt::allocation alloc = {size, key, true};
            _reporting_data.allocations()[ptr] = alloc;

            _reporting_data.stats().max_num_allocations = LMAX(_reporting_data.stats().max_num_allocations, _reporting_data.allocations().size());
            _reporting_data.stats().max_num_stacks      = LMAX(_reporting_data.stats().max_num_stacks,      _reporting_data.stacks().size());

            if (size == 0) {
                 ::error(ptr, "Zero bytes allocation", __FILE__, __LINE__);
            }
        }//lock
    }
    else {
        serror(ptr, "Untraced alloc", __FILE__, __LINE__);
    }
}

void free(__ptr_t ptr, memsize_type size)
{
    lpt::gnu_atomic_guard<volatile bool> let_me_in(&_in_trace, false, true);
    if (let_me_in.acquired())
    {
        //stack_mem_stats_type stack(true);

        {//lock
            std::unique_lock<std::mutex> lock(_reporting_lock);

            lpt::stack::key_type key = 0L;
            memsize_type nfree = 0L;
            lpt::allocation_map_type::iterator itAlloc = _reporting_data.allocations().find(ptr);
            if (itAlloc == _reporting_data.allocations().end()) {
                ::error(ptr, "Unknown free address", __FILE__, __LINE__);
            }
            else {
                key = itAlloc->second.key;
                nfree = itAlloc->second.num_bytes;
                //assert(size == itAlloc->second.num_bytes);
                _reporting_data.allocations().erase(itAlloc);
            }

            if (key) {
                stack_mem_stats_map_type::iterator itStack = _reporting_data.stacks().find(key);
                if (itStack == _reporting_data.stacks().end()) {
                    ::error(ptr, "Internal error", __FILE__, __LINE__);
                }
                else {
                    itStack->second.num_allocs--;
                    itStack->second.num_bytes -= nfree;
                    assert(itStack->second.num_allocs >= 0 && itStack->second.num_bytes >= 0L);
                    if (itStack->second.num_allocs == 0) {
                        _reporting_data.stacks().erase(itStack);
                    }
                }
            }
        }//lock
    }
    else {
        serror(ptr, "Untraced free", __FILE__, __LINE__);
    }
}

void realloc(__ptr_t newptr, __ptr_t oldptr, memsize_type size)
{
    free(oldptr, 0);
    alloc(newptr, size);
}

void allign(__ptr_t ptr, memsize_type size)
{
    alloc(ptr, size);
}

void error(__ptr_t ptr, memsize_type size)
{
    ::error(ptr, "Internal error", __FILE__, __LINE__);
}

void report()
{
    lpt::gnu_atomic_guard<volatile bool> let_me_in(&_in_trace, false, true);
    if (let_me_in.acquired())
    {
        lpt::timing start;

        static pid_t _mypid = ::getpid();
        static unsigned int _rptnum = 0L;
        static char _rptname[FILENAME_MAX + 1] = {0};

        {//lock
            std::unique_lock<std::mutex> lock(_reporting_lock);

            _rptnum++;
            ::snprintf(_rptname, FILENAME_MAX, "memleak.%d.delta-%d.rpt", _mypid, _rptnum);
            
            std::ofstream  rpt(_rptname, std::ofstream::out | std::ofstream::app);
            
            rpt << "Memory leaks report\n";


            std::vector<pdelta> summary;
            summary.resize(_reporting_data.stacks().size());
			
            mem_per_frame_map_type frames_mem;

            memsize_type total_bytes = 0L;
            for (const auto& stk : _reporting_data.stacks()) {
                total_bytes += stk.second.num_bytes;
                if (stk.second.num_bytes != stk.second.prev_num_bytes) {
                    pdelta delta = {0};
                    delta.num_bytes  = stk.second.num_bytes;
                    delta.num_allocs = stk.second.num_allocs;
                    delta.delta      = stk.second.num_bytes - stk.second.prev_num_bytes;
                    delta.key        = stk.first;
                    summary.emplace_back(delta);
                }
				
                call_stack_info_type::const_iterator itFrame= stk.second.begin();
                for (; itFrame != stk.second.end(); ++itFrame) {
                    frames_mem[itFrame->addr()] += stk.second.num_bytes;
                }
            }

            std::sort(summary.begin(), 
                      summary.end(), 
                      [](const pdelta& lhs, const pdelta& rhs) {
                           return lhs.num_bytes > rhs.num_bytes;
                      });
            rpt <<  "\nLeaks since previous report\n"
                    "======================================\n\n"
                    "StackKey, NumTotalBytes, NumAllocs, NumDeltaBytes\n"
                ;    
            for (const auto& delta: summary) {
                if (delta.key && delta.delta) {
                    rpt << std::hex << delta.key << ", "
                        << std::dec << delta.num_bytes << ", "
                        << delta.num_allocs << ", "
                        << delta.delta
                        ;
                }
            }
            rpt << "\n\n" << total_bytes << " total bytes, delta " << total_bytes - _reporting_data.stats().total_bytes << "\n";
            _reporting_data.stats().total_bytes = total_bytes;
            rpt << "Max tracked: stacks=" << _reporting_data.stats().max_num_stacks << ", allocations=" << _reporting_data.stats().max_num_allocations <<"\n\n";



            rpt <<  "\nAll known allocations\n"
                    "======================================\n\n"
                ;    
            for (auto& stk : _reporting_data.stacks()) {
                    rpt << std::hex << stk.first << ": " 
                        << std::dec << stk.second.num_bytes << " bytes in " 
                        << stk.second.num_allocs << " allocations \n"
                        ;
            }
            for (auto& stk : _reporting_data.stacks()) {
                    rpt << "\n\n" << std::hex << stk.first << ": " 
                        << std::dec << stk.second.num_bytes << " bytes in " 
                        << stk.second.num_allocs << " allocations \n"
                        ;
                    rpt << call_stack_info_type(stk.second);

                    stk.second.prev_num_bytes = stk.second.num_bytes;
            }


            rpt <<  "\n\nLeak per stack frame\n"
                    "--------------------------------------\n"
                ;  
            std::vector<mem_per_frame_type> mem_per_frame;
			mem_per_frame.resize(frames_mem.size());
            for (const auto& fr: frames_mem) {
			    mem_per_frame.emplace_back(fr);
            } //for
			
            std::sort(mem_per_frame.begin(), 
                      mem_per_frame.end(), 
                      [](const mem_per_frame_type& lhs, const mem_per_frame_type& rhs) {
                           return lhs.second > rhs.second;
                      });
            for (const auto& fr: mem_per_frame) {
                    frame_info_type frm(fr.first);

                    if (frm.addr() == lpt::stack::null_address_type) {
                        continue;
                    }

                    rpt << "\n" << std::hex << fr.first << ": " 
					    << std::dec << fr.second << " bytes \n"
                        ;
                    lpt::stack::fancy_formatter::print(frm, rpt);
                    rpt << "\n";
            } //for


            rpt <<  "\n\nAddress, StackKey, Bytes\n"
                    "--------------------------------------\n"
                ;    
            for (const auto& alloc: _reporting_data.allocations()) {
                rpt << std::hex << alloc.first << ", "
                    << std::hex << alloc.second.key << ", "
                    << std::dec << alloc.second.num_bytes << "\n"
                    ;
            } //for

            rpt <<  "\n\nThis report took " << start.elapsed<lpt::timing::milliseconds>() << " ms to generate.\n";
        }//lock
    }
    else {
        serror(nullptr, "No report", __FILE__, __LINE__);
    }
}

} //namespace
