# lpt-tools
Various programming/debugging tools and scripts

* A [PAPI](https://icl.utk.edu/papi/) wrapper to measure performance via the CPU counters. 
  Usage: see examples/papimove*.cpp
* Callstack tools per [N3441](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3441.html). See also [Boost-Call_stack].(https://github.com/melintea/Boost-Call_stack). Dependencies
  - libbfd (Debian: binutils-dev)
* Varia:
  * Compiler trick instructions
  * std::tuple for_each
  * enum::to_string
* libmemleak: an interposition library to detect memory leaks; suggestion: use [bcc](https://github.com/iovisor/bcc)
* (deprecated) libmtrace: an mtrace(3) interposition library 
