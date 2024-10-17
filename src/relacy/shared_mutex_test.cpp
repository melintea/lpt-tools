//
// shared_mutex test; bug free.
//

#include <relacy/relacy.hpp>
//#include <relacy/relacy_std.hpp>

#include "shared_mutex.h"

// template parameter '2' is number of threads
struct race_test : rl::test_suite<race_test, 2>
{
    std::shared_mutex smtx;
    rl::var<int> x;

    // executed in single thread before main thread function
    void before()
    {
        x($) = 0;
    }

    // main thread function
    void thread(unsigned thread_index)
    {
        if (0 == thread_index) // reader
        {
            int y = 100;
            {
                std::shared_lock rlock(smtx);
                y = x($);
            }
            RL_ASSERT(y == 2 || y == 0);
        }
        else // writer
        {
            std::unique_lock wlock(smtx);
            RL_ASSERT(0 == x($));
            if (0 == x($))
            {
                x($) = 2;
            }
        }
    }

    // executed in single thread after main thread function
    void after()
    {
    }

    // executed in single thread after every 'visible' action in main threads
    // disallowed to modify any state
    void invariant()
    {
    }
};

int main()
{
    rl::test_params params;
    //params.search_type = rl::sched_full;
    //params.iteration_count = 100000000;
    params.search_type = rl::sched_random;
    params.iteration_count = 1000000;
    
    rl::simulate<race_test>(/*params*/);
}

