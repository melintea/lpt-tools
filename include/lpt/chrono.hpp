/*
 *  $Id: $
 *
 *  Copyright 2023 Aurelian Melinte. 
 *  Released under LGPL 3.0 or later. 
 * 
 *  Measure elapsed time.
 */

#pragma once 

#include <chrono>
#include <functional>
#include <string>

#include <unistd.h>  // _POSIX_TIMERS

using namespace std::string_literals;

namespace lpt { namespace chrono {

struct timepoint_base
{
    using clock_t     = std::chrono::steady_clock;
    using timepoint_t = clock_t::time_point;
    using duration_t  = std::chrono::nanoseconds;
    using percent_t   = double;

    static constexpr const uint64_t NANOSECS = 1'000'000'000L;

    static const std::string& unit()
    {
        static const std::string ns{"ns"};
        return ns;
    }

    static const std::string& name()
    {
        static const std::string timeLabel = "ElapsedTime("s + unit() + ")"s;
        return timeLabel;
    }

}; // timepoint_base

class std_timepoint : public timepoint_base
{
public:

    std_timepoint() : _point(clock_t::now()) {}

    ~std_timepoint()                               = default;
    std_timepoint(const std_timepoint&)            = default;
    std_timepoint& operator=(const std_timepoint&) = default;
    std_timepoint(std_timepoint&&)                 = default;
    std_timepoint& operator=(std_timepoint&&)      = default;

    auto elapsed() const
    {
        return std::chrono::duration_cast<duration_t>(clock_t::now() - _point);
    }

    static percent_t as_percent_of(duration_t dataPoint, duration_t baseDuration)
    {
        return ((dataPoint - baseDuration)/(baseDuration*(percent_t)1.0)) * 100.0;
    }

    percent_t as_percent_of(duration_t baseDuration) const
    {
        return as_percent_of(elapsed(), baseDuration);
    }

protected:

    const timepoint_t       _point;
}; // std_timepoint

// About the same precision as std_timepoint
class posix_timepoint : public timepoint_base
{
public:

    posix_timepoint() 
    {
        ::clock_gettime(CLOCK_MONOTONIC, &_start);
    }

    ~posix_timepoint()                                 = default;
    posix_timepoint(const posix_timepoint&)            = default;
    posix_timepoint& operator=(const posix_timepoint&) = default;
    posix_timepoint(posix_timepoint&&)                 = default;
    posix_timepoint& operator=(posix_timepoint&&)      = default;

    auto elapsed() const
    {
        struct timespec now;
        ::clock_gettime(CLOCK_MONOTONIC, &now);

        uint64_t nanoDiff(NANOSECS*(now.tv_sec - _start.tv_sec) + (now.tv_nsec - _start.tv_nsec));
        return duration_t(nanoDiff);
    }

    static percent_t as_percent_of(duration_t dataPoint, duration_t baseDuration)
    {
        return ((dataPoint - baseDuration)/(baseDuration*(percent_t)1.0)) * 100.0;
    }

    percent_t as_percent_of(duration_t baseDuration) const
    {
        return as_percent_of(elapsed(), baseDuration);
    }

protected:

    struct timespec         _start;
}; // std_timepoint

#ifdef _POSIX_TIMERS
using timepoint = std_timepoint; //posix_timepoint;
#else
using timepoint = std_timepoint;
#endif  // _POSIX_TIMERS


// Do nothing functor
static void noop(const std::string& /*tag*/, const timepoint::duration_t& /*dur*/) noexcept {}
template <typename FUNC>
class measurement 
{
public:

    /// Apply the @param func functor in the destructor
    measurement(std::string      tag,
                FUNC             func)
        : _tag(std::move(tag))
        , _func(std::move(func))
    { }

    ~measurement()
    {
        auto tdiff(_start.elapsed());
        _func(_tag, tdiff);
    }

    measurement(const measurement&)            = delete;
    measurement& operator=(const measurement&) = delete;
    measurement(measurement&&)                 = default;
    measurement& operator=(measurement&&)      = default;

    auto elapsed() const
    {
        return _start.elapsed();
    }

private:

    const std::string       _tag;
    const timepoint         _start;
    FUNC                    _func;

}; // measurement

/**
 \code
    {
        using namespace std::chrono_literals;
        lpt::chrono::accumulator acc("Total time");
        {
            lpt::chrono::measurement m1(acc.measurement());
            std::this_thread::sleep_for(1230ms);
        }
        {
            lpt::chrono::measurement m2(acc.measurement());
            std::this_thread::sleep_for(3210ms);
        }
        std::cout << acc << std::endl; // 4440'000'000 ?
    }
 \endcode
*/
class accumulator 
{
public:

    accumulator(std::string tag)
        : _tag(std::move(tag))
    { }

    ~accumulator()                             = default;

    accumulator(const accumulator&)            = default;
    accumulator& operator=(const accumulator&) = default;
    accumulator(accumulator&&)                 = default;
    accumulator& operator=(accumulator&&)      = default;

    timepoint::duration_t elapsed() const
    {
        return _elapsed;
    }

    const std::string& tag() const
    {
        return _tag;
    }

    auto measurement() 
    {
        return lpt::chrono::measurement(_tag,
                                        [& acc=*this](auto&& /*tag*/, auto&& dur) -> decltype(auto) {
                                            acc += dur;
                                        });
    }

    accumulator& operator+=(const timepoint::duration_t& dur)
    {
        _elapsed += dur;
        return *this;
    }

    accumulator& operator+=(const timepoint& tp)
    {
        _elapsed += tp.elapsed();
        return *this;
    }

    template <typename FUNC>
    accumulator& operator+=(const class measurement<FUNC>& m) // NOLINT
    {
        _elapsed += m.elapsed(); // NOLINT
        return *this;
    }

    accumulator& operator+=(const accumulator& rhs)
    {
        _elapsed += rhs.elapsed();
        return *this;
    }

    friend std::ostream& operator<<(std::ostream&      os,
                                    const accumulator& acc)
    {
        os << acc._tag << ": "s 
           << std::to_string(acc._elapsed.count()) << " "s << timepoint::unit();
        return os;
    }

private:

    const std::string       _tag;
    timepoint::duration_t   _elapsed;

}; // accumulator

}} //namespace lpt::chrono

