/*
 *  $Id: $
 *
 *  Copyright 2024 Aurelian Melinte. 
 *  Released under LGPL 3.0 or later. 
 * 
 *  Time measurement stats container
 */

#pragma once

#include <lpt/chrono.hpp>
#include <lpt/stats.hpp>

#include <type_traits>

namespace lpt::chrono
{

struct dataset : public lpt::stats::dataset
{  
    using value_t = lpt::stats::dataset::value_t;
    static_assert(std::is_same_v<value_t, lpt::chrono::timepoint::percent_t>, "Both should be double");

    using lpt::stats::dataset::dataset;

    void operator()(const lpt::chrono::timepoint& data)
    {
        _stats(static_cast<value_t>(data.elapsed().count()));
    }

    void operator()(const lpt::chrono::timepoint::duration_t& data)
    {
        _stats(static_cast<value_t>(data.count()));
    }

    /// As percents over @param base
    void operator()(const lpt::chrono::timepoint::duration_t& data,
                    const lpt::chrono::timepoint::duration_t& base)
    {
        _stats(lpt::chrono::timepoint::as_percent_of(data, base));
    }
};

} // namespace lpt::chrono

