//
// With -std=c++20. Buggy on purpose
//

#include <relacy/relacy.hpp>
//#include <relacy/relacy_std.hpp>

// template parameter '2' is number of threads
struct race_test : rl::test_suite<race_test, 2>
{
    rl::atomic<int> a; //std::atomic
    rl::var<int> x;

    // executed in single thread before main thread function
    void before()
    {
        a($) = 0;
        x($) = 0;
    }

    // main thread function
    void thread(unsigned thread_index)
    {
        if (0 == thread_index)
        {
            x($) = 1;
	    // bug: use rl::mo_relaxed aka std::memory_order_relaxed
	    // fix: use rl::mo_release aka rl::memory_order_release
            a($).store(1, rl::mo_relaxed);
        }
        else
        {
	    // bug: use rl::mo_relaxed aka std::memory_order_relaxed
	    // fix: use rl::mo_acquire aka rl::memory_order_acquire
            if (1 == a($).load(rl::mo_acquire))
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

