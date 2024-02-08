//#include <gperftools/profiler.h>

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
        //ProfilerStart("dump.txt");

        int a = 100;
	int tot = 0;
        while(a--){tot += foo();}
        cout << tot << endl;
        
	//ProfilerFlush();
        //ProfilerStop();
}

