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
 * 
 *  Needs to run as root
 */

#ifndef LPT_PAPI_H
#define LPT_PAPI_H

#pragma once

#include <lpt/chrono.hpp>

#include <array>
#include <algorithm>
#include <atomic>
#include <cassert>
#include <iostream>
#include <functional>
#include <new>         // std::hardware_constructive_interference_size
#include <stdexcept>
#include <string>
#include <utility>

#include <papi.h> 

#include <unistd.h>

#if (__cplusplus < 201703L)
#  error Minimum C++17 needed
#endif
#if (PAPI_VERSION < PAPI_VERSION_NUMBER(7,0,1,0))
#  error Needs minimum PAPI version(7,0,1,0)
#endif

using namespace std::string_literals;

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
#ifdef __cpp_lib_hardware_interference_size
        using std::hardware_constructive_interference_size;
        using std::hardware_destructive_interference_size;
#else
        constexpr std::size_t hardware_constructive_interference_size = 64;
        constexpr std::size_t hardware_destructive_interference_size  = 64;
#  warning Unknown hardware_constructive_interference_size, check values above
#endif
        os << "hardware_constructive_interference_size=" 
	   << hardware_constructive_interference_size 
	   << "\nhardware_destructive_interference_size="
	   << hardware_destructive_interference_size;

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

    friend inline std::ostream& operator<<(std::ostream&   os,
                                           const hardware& hw)
    {
        return hw.print(os);
    }

    hardware(const hardware&)            = delete;
    hardware& operator=(const hardware&) = delete;
    hardware(hardware&&)                 = delete;
    hardware& operator=(hardware&&)      = delete;

    constexpr int num_counters() const { return _numHwCtrs; }

    struct cpu_info
    {
        cpu_info(const hardware& hw)
            : _model(hw._hwInfo->model_string)
            , _numHwCtrs(hw._numHwCtrs)
        { }

        const char* _model;
        const int   _numHwCtrs;
    };
    const cpu_info cpu() const { return cpu_info(*this); }

private:

    friend cpu_info;

    int             _numHwCtrs{0};
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
    using event_t        = int;
    using events_t       = std::array<event_t, NUM_COUNTERS>;
    using eventset_t     = int;
    using name_t         = char[PAPI_MAX_STR_LEN + 1];
    using names_t        = std::array<std::string, NUM_COUNTERS>;
    using value_t        = long long;
    using values_t       = std::array<value_t, NUM_COUNTERS>;
    using eol_functor_t  = std::function<void(const counters&)>; // called in destructor


    /**
     *  @code
     *  counters scopedCtrs(
     *     "some data", 
     *     [](const counters& ctrs){
     *          std::cout << ctrs << std::endl;
     *     });
     *  @endcode
     */
    counters(std::string   tag     = {},
             eol_functor_t eolFunc = noop)
        : _tag(std::move(tag))
        , _eolFunc(std::move(eolFunc))
    {
        if (0 != ::geteuid()) {
            throw error("Must run with elevated priv", PAPI_EPERM);
        }

        thread::init();
        
        int retval{PAPI_OK};

        retval = PAPI_create_eventset(&_eventSet);
        if (retval != PAPI_OK) {
            throw error("PAPI_create_eventset", retval);
        }

        retval = PAPI_add_events(_eventSet, const_cast<event_t*>(_events.begin()), NUM_COUNTERS);
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
        int retval(PAPI_stop(_eventSet, discard.begin()));
        if (retval != PAPI_OK) {
            throw error("PAPI_stop", retval);
        }

        _eolFunc(*this);
    }

    counters(const counters&)            = delete;
    counters& operator=(const counters&) = delete;
    counters(counters&&)                 = delete;
    counters& operator=(counters&&)      = delete;

    static constexpr const size_t  size() { return NUM_COUNTERS; }

    const     events_t&    events()   const { return _events; }
    const     values_t&    values()   const { return _accumulators; }
              eventset_t   eventset() const { return _eventSet; }
    const     std::string& tag()      const { return _tag; }

    static std::string name(size_t idx)
    {
        static const names_t _names = [&](){
            names_t ns2;
            for (auto idx = 0; idx < size(); ++idx) {
                name_t n = {0};
                PAPI_event_code_to_name(_events[idx], n);
                ns2[idx] = n;

                //PAPI_event_info_t _einfo;
                //PAPI_get_event_info(evt, &_einfo);
            }
            return ns2;
        }();

        assert(idx < size());
        return _names[idx];
    }

    void accumulate(const values_t& vals)
    {
        for (size_t i = 0; i < size(); ++i )
        {
            _accumulators[i] += vals[i];
        }
    }

    std::ostream& print(std::ostream& os) const
    {
        if ( ! _tag.empty())
        {
            os << _tag << ": \n";
        }
        for (size_t i = 0; i < size(); ++i )
        {
            os << name(i) << ": " << _accumulators[i] << '\n';
        }

        return os;
    }

    friend inline std::ostream& operator<<(std::ostream&   os,
                                           const counters& ctrs)
    {
        return ctrs.print(os);
    }

    // Do nothing functor
    static void noop(const counters&) noexcept {}


    using percent_t      = double;


    struct datapoint 
    {
        std::string                         _tag;
        values_t                            _values{0};
        lpt::chrono::timepoint::duration_t  _elapsedTime{0}; 

        // FIXME: array is not really meant to be inheritable
        struct percents : public std::array<percent_t, counters::size() + 1/*_elapsedTime*/>
        {
            using typename std::array<percent_t, NUM_COUNTERS + 1/*_elapsedTime*/>::array;

            static constexpr const size_t POS_TIME = NUM_COUNTERS; // last entry in the array is for time

            static constexpr const size_t size() { return NUM_COUNTERS + 1; }

            static std::string name(size_t idx)
            {
                assert(idx < size());
                static const std::string timeLabel = "ElapsedTime("s + lpt::chrono::timepoint::unit() + ")"s;
                return idx < counters::size() ? counters::name(idx) : timeLabel;
            }

            std::ostream& print(std::ostream& os) const
            {
                for (size_t i = 0; i < size(); ++i )
                {
                    //std::cout << std::format("{:10} : {:.2f} %\n", counters::name(i), this->at(i) );
                    os << name(i) << ": " << this->at(i) << " % \n";
                }

                os << std::endl;

                return os;
            }

            friend inline std::ostream& operator<<(std::ostream&   os,
                                                const percents& pcts)
            {
                return pcts.print(os);
            }
        }; //percents

        datapoint(const std::string& tag,
                  const values_t&    values)
            : _tag(tag)
            //, _values(values)
        {           
            std::copy(std::begin(values), std::end(values), std::begin(_values));
        }

        datapoint(std::string tag)
            : _tag(std::move(tag))
        {           
        }

        datapoint()                            = default;
        datapoint(const datapoint&)            = default;
        datapoint& operator=(const datapoint&) = default;
        datapoint(datapoint&&)                 = default;
        datapoint& operator=(datapoint&&)      = default;

        constexpr size_t       size()   const { return counters::size(); }
        const     values_t&    values() const { return _values; }
        const     std::string& tag()    const { return _tag; }
        constexpr lpt::chrono::timepoint::duration_t  elapsed_time() const { return _elapsedTime; }

        datapoint operator-(const datapoint& r) const
        {
            datapoint ret;
            for (auto i = 0; i < size(); ++i )
            {
                ret._values[i] = _values[i] - r._values[i];
            }

            ret._elapsedTime = _elapsedTime - r._elapsedTime;

            return ret;
        }

        percents as_percent_of(const datapoint& base) const
        {
            percents pcts;
            for (auto i = 0; i < size(); ++i )
            {
                percent_t dataPoint(_values[i]);
                percent_t basePoint(base._values[i]);
                pcts[i] = basePoint != 0
                        ? (((dataPoint - basePoint)/basePoint) * 100.0)
                        : 0 ;
            }

            percent_t dataPoint(_elapsedTime.count());
            percent_t basePoint(base._elapsedTime.count());
            pcts[percents::POS_TIME] = ((dataPoint - basePoint)/basePoint) * 100.0;

            return pcts;
        }

        std::ostream& print(std::ostream& os) const
        {
            if ( ! _tag.empty())
            {
                os << _tag << ": \n";
            }
            for (size_t i = 0; i < NUM_COUNTERS; ++i )
            {
                os << counters::name(i) << ": " << _values[i] << '\n';
            }

            os << percents::name(percents::POS_TIME) << ": " << _elapsedTime.count() << '\n';

            os << std::endl;

            return os;
        }

        friend inline std::ostream& operator<<(std::ostream&           os,
                                               const datapoint& md)
        {
            return md.print(os);
        }
    }; // datapoint

    template <typename FUNC>
    class measurement : public datapoint
    {
    public:

        /// Apply the @param eolFunc functor in the destructor
        measurement(std::string      tag,
                    counters&        ctrs,
                    FUNC             eolFunc)
            : datapoint{std::move(tag)}
            , _startTime(lpt::chrono::timepoint::clock_t::now())
            , _counters(ctrs)
            , _eolFunc(std::move(eolFunc))
        {
            int retval(PAPI_read(_counters.eventset(), datapoint::_values.begin()));
            if (retval != PAPI_OK) {
                throw error("PAPI_read", retval);
            }
        }

        ~measurement()
        {
            values_t  _second_read{0};

            int retval(PAPI_read(_counters.eventset(), _second_read.begin()));
            if (retval == PAPI_OK)
            {
                for (auto i = 0; i < counters::size(); ++i )
                {
                    datapoint::_values[i] = _second_read[i] - datapoint::_values[i];
                }
                _counters.accumulate(datapoint::_values);

                datapoint::_elapsedTime = std::chrono::duration_cast<lpt::chrono::timepoint::duration_t>(lpt::chrono::timepoint::clock_t::now() - _startTime);

                _eolFunc(this);
            }
        }

        measurement(const measurement&)            = default;
        measurement& operator=(const measurement&) = default;
        measurement(measurement&&)                 = default;
        measurement& operator=(measurement&&)      = default;

        datapoint data() const
        {
            datapoint dnow{datapoint::_tag};

            int retval(PAPI_read(_counters.eventset(), dnow._values.begin()));
            if (retval != PAPI_OK) {
                throw error("PAPI_read", retval);
            }
                
            for (auto i = 0; i < counters::size(); ++i )
            {
                dnow._values[i] -= datapoint::_values[i];
            }

            dnow._elapsedTime =  lpt::chrono::timepoint::clock_t::now() - _startTime;

            return dnow;
        }

    private:

        counters&                            _counters;
        lpt::chrono::timepoint::timepoint_t  _startTime;
        FUNC                                 _eolFunc;

    }; // measurement

private:

    static constexpr const events_t   _events{ EVENTS... };
    //static constexpr const names_t    _names{};
    const std::string                 _tag;
    const eol_functor_t               _eolFunc{noop}; // called in destructor
    eventset_t                        _eventSet{PAPI_NULL};
    values_t                          _accumulators{0};

}; // counters


} // namespace lpt::papi

#endif // LPT_PAPI_H
