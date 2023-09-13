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

template <typename FUNC>
class measurement 
{
public:

    using clock_t     = std::chrono::steady_clock;
    using timepoint_t = clock_t::time_point;
    using duration_t  = std::chrono::nanoseconds;

    /// Apply the @param func functor in the destructor
    measurement(std::string      tag,
                FUNC             func)
        : _tag(std::move(tag))
        , _start(clock_t::now())
        , _func(std::move(func))
    { }

    ~measurement()
    {
        auto tdiff(elapsed());
        _func(_tag, tdiff);
    }

    measurement(const measurement&)            = default;
    measurement& operator=(const measurement&) = default;
    measurement(measurement&&)                 = default;
    measurement& operator=(measurement&&)      = default;

    auto elapsed() const
    {
        return std::chrono::duration_cast<duration_t>(clock_t::now() - _start);
    }

private:

    const std::string       _tag;
    const timepoint_t       _start;
    FUNC                    _func;

}; // measurement

}} //namespace lpt::chrono

