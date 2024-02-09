/*
 *  $Id: $
 *
 *  Copyright 2023 Aurelian Melinte. 
 *  Released under LGPL 3.0 or later. 
 * 
 *  Time measurement stats container
 */

#pragma once

#include <lpt/chrono.hpp>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/moment.hpp>
#include <boost/accumulators/statistics/variance.hpp>

namespace lpt::chrono
{

struct accumulator_set 
{  
    using percent_t         = timepoint::percent_t;
    using value_t           = timepoint::duration_t;

    using accumulator_set_t = boost::accumulators::accumulator_set< percent_t,
                                                                    boost::accumulators::features <
                                                                        boost::accumulators::tag::min
                                                                      , boost::accumulators::tag::max
                                                                      , boost::accumulators::tag::median
                                                                      , boost::accumulators::tag::mean
                                                                      , boost::accumulators::tag::variance
                                                                      , boost::accumulators::tag::count
                                                                    >
                                                                  >;

    void operator()(const percent_t& data)
    {
        _stats[0](data);
    }

    //void operator()(const timepoint& data)
    //{
    //    _stats[0](data.elapsed());
    //}

    friend std::ostream& operator<<(std::ostream& os, const accumulator_set& dt)
    {
        const auto& stat(dt._stats[0]);
        const auto  n(boost::accumulators::count(stat));
        os << boost::accumulators::count(stat) << " samples\n"
           << "Name, min%, max%, mean%, median%, stddev\n";
        os << timepoint::name()                 << ", "
           << boost::accumulators::min(stat)    << ", "
           << boost::accumulators::max(stat)    << ", "
           << boost::accumulators::mean(stat)   << ", "
           << boost::accumulators::median(stat) << ", "
           << std::sqrt(boost::accumulators::variance(stat) * (n/(n-1.0)))
           << '\n';

        return os;
    }

    accumulator_set_t _stats[1];
};

} // namespace lpt::chrono

