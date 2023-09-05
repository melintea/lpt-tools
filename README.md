# lpt-tools
Various programming/debugging tools and scripts

* A [PAPI](https://icl.utk.edu/papi/) wrapper to measure performance via the CPU counters. 
  Usage: see examples/papimove*.cpp
* Callstack tools per [N3441](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3441.html). See also [Boost-Call_stack].(https://github.com/melintea/Boost-Call_stack). Dependencies
  - libbfd (Debian: binutils-dev)
* Varia/compiler trick instructions
* (deprecated) libmemleak: an interposition library to detect memory leaks
* (deprecated) libmtrace: an mtrace(3) interposition library 
