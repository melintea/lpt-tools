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

#pragma once

#include <lpt/papi/papi.hpp>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/moment.hpp>
#include <boost/accumulators/statistics/variance.hpp>

namespace lpt::papi
{

template<typename PAPI_COUNTERS> 
struct accumulator_set 
{  
    using counters_t        = PAPI_COUNTERS;
    using percent_t         = typename counters_t::percent_t;
    using percents_t        = typename counters_t::datapoint::percents;

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

    void operator()(const percents_t& data)
    {
        for (auto i = 0; i < percents_t::size(); ++i) {
            _stats[i](data[i]);
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const accumulator_set& dt)
    {
        os << boost::accumulators::count(dt._stats[0]) << " samples\n"
           << "Counter, min%, max%, mean%, median%, stddev\n";
        for (auto i = 0; i < percents_t::size(); ++i) {
            const auto& stat(dt._stats[i]);
            const auto  n(boost::accumulators::count(stat));
            os << percents_t::name(i)               << ", "
               << boost::accumulators::min(stat)    << ", "
               << boost::accumulators::max(stat)    << ", "
               << boost::accumulators::mean(stat)   << ", "
               << boost::accumulators::median(stat) << ", "
               << std::sqrt(boost::accumulators::variance(stat) * (n/(n-1.0)))
               << '\n';
        }

        return os;
    }

    accumulator_set_t _stats[percents_t::size()];
};

} // namespace lpt::papi

