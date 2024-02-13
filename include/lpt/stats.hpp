/*
 *  $Id: $
 *
 *  Copyright 2024 Aurelian Melinte. 
 *  Released under LGPL 3.0 or later. 
 * 
 *  Basic stats container
 */

#pragma once

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/moment.hpp>
#include <boost/accumulators/statistics/variance.hpp>

#include <string>

namespace lpt::stats
{

struct dataset 
{  
    using value_t   = double;

    using dataset_t = boost::accumulators::accumulator_set< value_t,
                                                            boost::accumulators::features <
                                                                boost::accumulators::tag::min
                                                                , boost::accumulators::tag::max
                                                                , boost::accumulators::tag::median
                                                                , boost::accumulators::tag::mean
                                                                , boost::accumulators::tag::variance
                                                                , boost::accumulators::tag::count
                                                            >
                                                          >;

    dataset(const std::string& tag) : _tag(tag) {}

    dataset()                          = default;
    ~dataset()                         = default;
    dataset(const dataset&)            = default;
    dataset& operator=(const dataset&) = default;
    dataset(dataset&&)                 = default;
    dataset& operator=(dataset&&)      = default;

    void operator()(const value_t& data)
    {
        _stats(data);
    }

    friend std::ostream& operator<<(std::ostream& os, const dataset& dt)
    {
        const auto& stat(dt._stats);
        const auto  n(boost::accumulators::count(stat));
        os << boost::accumulators::count(stat) << " samples:\n"
           << "Name, min%, max%, mean%, median%, stddev\n";
        os << dt._tag                           << ", "
           << boost::accumulators::min(stat)    << ", "
           << boost::accumulators::max(stat)    << ", "
           << boost::accumulators::mean(stat)   << ", "
           << boost::accumulators::median(stat) << ", "
           << std::sqrt(boost::accumulators::variance(stat) * (n/(n-1.0)))
           << '\n';

        return os;
    }

    const std::string _tag;
    dataset_t         _stats;
};

} // namespace lpt::stats

