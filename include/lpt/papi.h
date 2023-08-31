/*
 *  $Id: $
 *
 *  Copyright 2023 Aurelian Melinte. 
 *  Released under LGPL 3.0 or later. 
 * 
 *  PAPI tools.
 *
 *  Needs PAPI installed - see INSTALL.txt:
 *   ./configure && make && sudo make install-all
 */

#ifndef LPT_PAPI_H
#define LPT_PAPI_H

#pragma once

#include <iostream>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <cassert>
#include <atomic>

#include <papi.h> 

#if (__cplusplus < 201703L)
#  error Minimum C++17 needed
#endif
#if (PAPI_VERSION < PAPI_VERSION_NUMBER(7,0,1,0))
#  error Needs minimum PAPI version(7,0,1,0)
#endif

namespace lpt::papi
{

/*
 *
 */

class error : public std::runtime_error
{
public: 

    explicit error(std::string const& msg, int err) 
        : std::runtime_error(msg)
        , _msg(msg)
        , _err(err)
    {
        _msg += ": ";
        _msg += PAPI_strerror(_err);
    }
    
    error(error&& other) noexcept
        : std::runtime_error(std::move(other._msg.c_str()))
        , _msg(std::move(other._msg))
        , _err(other._err)
    {
        ///swap(other);
    }
    
    error& operator=(error other) noexcept
    {
        swap(other);
        return *this;
    }
    
    error& operator=(error&& other) noexcept
    {
        ///swap(other);
        std::runtime_error::operator=(std::move(other));
        _msg = std::move(other._msg); 
        _err = other._err; 
        return *this;
    }
    
   friend std::ostream& operator<< (std::ostream& os, error const& err);

   virtual ~error() noexcept {}
    
    const char *what() const noexcept
    {
        return _msg.c_str();
    }

private: 

    //FIXME: namespace level swap
    void swap(error& other) noexcept
    {
        std::swap(_msg, other._msg);
        _err = other._err;
    }

private: 

    std::string _msg;
    int         _err;

}; // error

inline std::ostream& operator<< (std::ostream& os, error const& err)
{
    os << err.what();
    return os;
}


/*
 *
 */

class library
{
public:

    static bool init()
    {
        static bool once = []{
            int retval      = PAPI_OK;

            if ((retval = PAPI_library_init(PAPI_VER_CURRENT)) != PAPI_VER_CURRENT ) {
                throw error("PAPI_library_init", retval);
            }
        
            // Must be called only once, just after PAPI_library_init
            if ( (retval = PAPI_thread_init( (unsigned long (*)(void))(pthread_self) )) != PAPI_OK) {
                throw error("PAPI_thread_init", retval);
            }
           
#if 0
            int retval = PAPI_OK;
            if ( (retval = PAPI_multiplex_init()) != PAPI_OK) {
                throw error("PAPI_multiplex_init", retval);
            }
#endif           

            return true;
        }();

        return once;
    }

    static void fini()
    {
        std::cout << "library::fini\n";

        //FIXME:
        //  PAPI_cleanup_eventset
        //  PAPI_shutdown
    }
    
}; // library


/*
 *
 */

class thread
{
public:

    static void init()
    {
        static thread_local unsigned long  tid = []{
            library::init();
            
            int retval = PAPI_OK;

            /*
             * Cf. PAPI Users Manual, unbounded threads use per process counters.
             * Still...
             */
            if ( (retval = PAPI_register_thread()) != PAPI_OK) {
                throw error("PAPI_register_thread", retval);
            }

            return PAPI_thread_id();        
        }();
    }

    static void fini()
    {
        std::cout << "thread::fini\n";
        //FIXME:
        //  PAPI_stop
        //  PAPI_cleanup_eventset
        //  PAPI_unregister_thread
    }
    
}; // thread


/*
 *
 */

class hardware
{
public:

    hardware()
    {
       thread::init();

       int retval{PAPI_OK};

        /*
         * PAPI_num_hwcounters returns the number of hardware counters the platform
         * has or a negative number if there is an error. 
         */
        if ((_numHwCtrs = PAPI_num_cmp_hwctrs(0)) < PAPI_OK) {
            throw error("PAPI_num_cmp_hwctrs", retval);
        }

        _hwInfo = const_cast<decltype(_hwInfo)>(PAPI_get_hardware_info());
        if (_hwInfo == NULL) {
            throw error("PAPI_get_hardware_info", PAPI_ESYS);
        }
    }

    std::ostream& print(std::ostream& os) const
    {
        os <<  "There are " << _numHwCtrs 
           << " counters for " << _hwInfo->vendor_string 
          << ":" << _hwInfo->model_string 
          << std::endl;
        os <<  "Cache: " << _hwInfo->mem_hierarchy.levels << " levels\n";
        for (int i = 0; i < _hwInfo->mem_hierarchy.levels; ++i) {
            const PAPI_mh_level_t& level = _hwInfo->mem_hierarchy.level[i];
            os << " " << i+1 << ".1:  size=" << level.cache[0].size 
               << " " << level.cache[0].num_lines << " lines * " 
               << level.cache[0].line_size << " assoc " 
               << level.cache[0].associativity << "\n"
               ;
            os << " " << i+1 << ".1:  size=" << level.cache[1].size 
               << " " << level.cache[1].num_lines << " lines * " 
               << level.cache[1].line_size << " assoc " 
               << level.cache[1].associativity << "\n"
               ;
        }
        os << std::endl;

        return os;
    }

    hardware(const hardware&)            = delete;
    hardware& operator=(const hardware&) = delete;
    hardware(hardware&&)                 = delete;
    hardware& operator=(hardware&&)      = delete;

private:

    int            _numHwCtrs{0};
    PAPI_hw_info_t* _hwInfo{nullptr};

}; // hardware

/*
 *
 */

template <int... EVENTS> 
class counters
{
public:

    static constexpr const size_t NUM_COUNTERS = sizeof...(EVENTS);
    using event_t    = int;
    using value_t    = long long;
    using events_t   = event_t[NUM_COUNTERS];
    using eventset_t = int;
    using values_t   = value_t[NUM_COUNTERS];

    counters()
    {
        thread::init();
        
        int retval{PAPI_OK};

        retval = PAPI_create_eventset(&_eventSet);
        if (retval != PAPI_OK) {
            throw error("PAPI_create_eventset", retval);
        }

        retval = PAPI_add_events(_eventSet, (int*)_events, NUM_COUNTERS);
        if (retval != PAPI_OK) {
            // events that can be added are added even if it returns an error
            //throw error("PAPI_add_events", retval);
        }

        retval = PAPI_start(_eventSet);
        if (retval != PAPI_OK) {
            throw error("PAPI_start", retval);
        }
#if 0
        retval = PAPI_set_multiplex(_eventSet);
        if (retval != PAPI_OK) {
            throw error("PAPI_set_multiplex", retval);
        }
#endif
    }

    ~counters() noexcept(false)
    {
        values_t  discard{0};
        int retval(PAPI_stop(_eventSet, discard));
        if (retval != PAPI_OK) {
            throw error("PAPI_stop", retval);
        }
    }

    counters(const counters&)            = delete;
    counters& operator=(const counters&) = delete;
    counters(counters&&)                 = delete;
    counters& operator=(counters&&)      = delete;

    constexpr size_t     size()     const { return NUM_COUNTERS; }
    const     events_t&  events()   const { return _events; }
    const     values_t&  values()   const { return _accumulators; }
              eventset_t eventset() const { return _eventSet; }

    static std::string name(size_t idx)
    {
        assert(idx < NUM_COUNTERS);

        char  n[PAPI_MAX_STR_LEN+1] = {0};
        PAPI_event_code_to_name(_events[idx], n);
        return n;

        //PAPI_event_info_t _einfo;
        //PAPI_get_event_info(evt, &_einfo);
    }

    void accumulate(const values_t& vals)
    {
        for (size_t i = 0; i < NUM_COUNTERS; ++i )
        {
            _accumulators[i] += vals[i];
        }
    }

    std::ostream& print(std::ostream& os) const
    {
        for (size_t i = 0; i < NUM_COUNTERS; ++i )
        {
            os << name(i) << ": " << _accumulators[i] << '\n';
        }

        return os;
    }

    struct measurement_data
    {
        std::string   _tag;
        values_t      _values{0};

        measurement_data(const std::string& tag,
                         const values_t&    values)
            : _tag(tag)
            //, _values(values)
        {           
            std::copy(std::begin(values), std::end(values), std::begin(_values));
        }

        measurement_data(std::string tag)
            : _tag(std::move(tag))
        {           
        }

        measurement_data()                                   = default;
        measurement_data(const measurement_data&)            = default;
        measurement_data& operator=(const measurement_data&) = default;
        measurement_data(measurement_data&&)                 = default;
        measurement_data& operator=(measurement_data&&)      = default;

        constexpr size_t       size()   const { return NUM_COUNTERS; }
        const     values_t&    values() const { return _values; }
        const     std::string& tag()    const { return _tag; }

        measurement_data operator-(const measurement_data& r)
        {
            measurement_data ret;
            for (auto i = 0; i < NUM_COUNTERS; ++i )
            {
                ret._values[i] = _values[i] - r._values[i];
            }
            return ret;
        }

    }; // measurement_data

    template <typename FUNC>
    class measurement : public measurement_data
    {
    public:

        measurement(std::string      tag,
                    counters&        ctrs,
                    FUNC             func)
            : measurement_data{std::move(tag)}
            , _counters(ctrs)
            , _func(std::move(func))
        {
            int retval(PAPI_read(_counters.eventset(), measurement_data::_values));
            if (retval != PAPI_OK) {
                throw error("PAPI_read", retval);
            }
        }

        ~measurement()
        {
            values_t  _second_read{0};

            int retval(PAPI_read(_counters.eventset(), _second_read));
            if (retval == PAPI_OK)
            {
                for (auto i = 0; i < NUM_COUNTERS; ++i )
                {
                    measurement_data::_values[i] = _second_read[i] - measurement_data::_values[i];
                }
                _counters.accumulate(measurement_data::_values);
                _func(this);
            }
        }

        measurement(const measurement&)            = default;
        measurement& operator=(const measurement&) = default;
        measurement(measurement&&)                 = default;
        measurement& operator=(measurement&&)      = default;

        measurement_data data() const
        {
            measurement_data dnow{measurement_data::_tag};

            int retval(PAPI_read(_counters.eventset(), dnow._values));
            if (retval != PAPI_OK) {
                throw error("PAPI_read", retval);
            }
                
            for (auto i = 0; i < NUM_COUNTERS; ++i )
            {
                dnow._values[i] -= measurement_data::_values[i];
            }

            return dnow;
        }

    private:

        counters&         _counters;
        FUNC              _func;

    }; // measurement

private:

    static constexpr const events_t   _events{ EVENTS... };
    eventset_t                        _eventSet{PAPI_NULL};
    values_t                          _accumulators{0};

}; // counters


} // namespace lpt::papi

#endif // LPT_PAPI_H
