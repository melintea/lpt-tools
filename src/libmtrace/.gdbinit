set env MALLOC_TRACE=./logs/mtrace.debug.log
set env LD_PRELOAD=./libmtrace.so

set breakpoint pending on
b demangle
b print_address
b print_stack

run
