/*
 *  $Id: $
 *
 *  Copyright 2023 Aurelian Melinte. 
 *  Released under LGPL 3.0 or later. 
 * 
 *  Various measurements using PAPI.
 *
 */


#include <lpt/papi/papi.hpp>

const int nlines = 196608;
const int ncols  = 64;
char ctrash[nlines][ncols];


//------------------------------------------------------------------------------
#define THRESHOLD 10000

void computation_mult()
{
   double tmp=1.0;
   int i=1;
   for( i = 1; i < THRESHOLD; i++ )
   {
      tmp = tmp*i;
   }
}

void computation_add()
{
   int tmp = 0;
   int i=0;

   for( i = 0; i < THRESHOLD; i++ )
   {
      tmp = tmp + i;
   }

}

//------------------------------------------------------------------------------


int main()
{
   std::cout << "*\n"
                "* Hardware: \n"
                "*\n";
   lpt::papi::hardware().print(std::cout);

   using counters = lpt::papi::counters<
         PAPI_TOT_INS // Total instructions"
       , PAPI_TOT_CYC // "Total cpu cycles"
       , PAPI_L1_DCM  // "L1 load  misses"
       // , PAPI_L1_STM  // "L1 store  missess"
       , PAPI_L2_DCM  // "L2 load  misses"
       // , PAPI_L2_STM  // "L2 store  missess"
       , PAPI_BR_MSP  // "Branch mispredictions"
   >;

   counters ctrs;
   auto cout_measurement = [](const counters::measurement_data* measure) -> void {
                                  const auto& vals(measure->values());
                                  std::cout << measure->tag() << '\n';
                                  for (auto i = 0; i < measure->size(); ++i) {
                                      std::cout << counters::name(i) << ": " << vals[i] << '\n';
                                  }
                                  std::cout << std::endl;
                            };

   std::cout << "*\n"
                "* Baseline \n"
                "*\n";
   {
       counters::measurement pc("Baseline add()",
                                ctrs,
                                cout_measurement);
       computation_add();
   }
   {
       counters::measurement pc("Baseline mult()", ctrs, cout_measurement);
       computation_mult();
   }
       
   std::cout << "*\n"
                "* add & mult \n"
                "*\n";
   {
       counters::measurement pc("add+mult no intermediate print", ctrs, cout_measurement);
       
       {
           counters::measurement pc2("add & no print", ctrs, cout_measurement);
           computation_add();
       }
       
       {
           counters::measurement pc2("mult & no print", ctrs, cout_measurement);
           computation_mult();
       }
   }
   {
       counters::measurement pc("add+mult & print", ctrs, cout_measurement);
       
       {
           counters::measurement pc2("add & print", ctrs, cout_measurement);
           computation_add();
       }
       
       {
           counters::measurement pc2("mult & print", ctrs, cout_measurement);
           computation_mult();
       }
   }
    
   std::cout << "*\n"
                "* Atomics \n"
                "*\n";
   {
       int i = 0;
       counters::measurement pc("Plain int", ctrs, cout_measurement);
       for (int i = 0; i < THRESHOLD; ++i) {
           i = i++;
       }
   }
   {
       std::atomic<int> i(0);
       counters::measurement pc("Atomic int", ctrs, cout_measurement);
       for (int i = 0; i < THRESHOLD; ++i) {
           i++;
       }
   }
   
   std::cout << "*\n"
                "* Data cache trashing \n"
                "*\n";
   {
       int x;
       counters::measurement pc("By line", ctrs, cout_measurement);
       for (int l = 0; l < nlines; ++l) {
           for (int c = 0; c < ncols; ++c) {
               x = ctrash[l][c];
           }
       }
   }
   {
       int x;
       counters::measurement pc("by column", ctrs, cout_measurement);
       for (int c = 0; c < ncols; ++c) {
           for (int l = 0; l < nlines; ++l) {
               x = ctrash[l][c];
           }
       }
   }
   
   std::cout << "*\n"
                "* Data cache trashing - placate optimizer \n"
                "*\n";
   {
       int x;
       counters::measurement pc("by column", ctrs, cout_measurement);
       for (int c = 0; c < ncols; ++c) {
           for (int l = 0; l < nlines; ++l) {
               x = ctrash[l][c];
               ctrash[l][c] = x + 1;
           }
       }
   }
   {
       int x;
       counters::measurement pc("By line", ctrs, cout_measurement);
       for (int l = 0; l < nlines; ++l) {
           for (int c = 0; c < ncols; ++c) {
               x = ctrash[l][c];
               ctrash[l][c] = x + 1;
           }
       }
   }

   std::cout << "*\n"
                "* Accumulated data \n"
                "*\n";
    ctrs.print(std::cout);
    std::cout << std::endl;

   exit(EXIT_SUCCESS);    
}

