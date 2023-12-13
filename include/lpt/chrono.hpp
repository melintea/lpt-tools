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
#include <string>

namespace lpt { namespace chrono {

class timepoint 
{
public:

    using clock_t     = std::chrono::steady_clock;
    using timepoint_t = clock_t::time_point;
    using duration_t  = std::chrono::nanoseconds;

    timepoint() : _point(clock_t::now()) {}

    ~timepoint()                           = default;
    timepoint(const timepoint&)            = default;
    timepoint& operator=(const timepoint&) = default;
    timepoint(timepoint&&)                 = default;
    timepoint& operator=(timepoint&&)      = default;

    auto elapsed() const
    {
        return std::chrono::duration_cast<duration_t>(clock_t::now() - _point);
    }

protected:

    const timepoint_t       _point;
};

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

    measurement(const measurement&)            = default;
    measurement& operator=(const measurement&) = default;
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

}} //namespace lpt::chrono

