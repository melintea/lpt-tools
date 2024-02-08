#include <lpt/gperftools.hpp>

#include <iostream>
#include <math.h>

using namespace std;

int bar()
{
        int a,b,c,d,j,k;
        a=0;
        int z=0;
        b = 1000;
        while(z < b)
        {
                while (a < b)
                {
                        d = sin(a);
                        c = cos(a);
                        j = tan(a);
                        k = tan(a);
                        k = d * c + j *k;
                        a++;
                }
                a = 0;
                z++;
        }
	
	return a+b+c+d+j+k;
}

int foo()
{
	return bar();
}

int main()
{
    {
        std::cout << "File: " << ::getenv("LPT_CPUPROFILE") << std::endl;
        lpt::gperftools::cpu_profiler prof;

        int a = 100;
	int tot = 0;
        while(a--){tot += foo();}
        std::cout << tot << std::endl;
    }
}

