#include <iostream>
#include <exception>
#include <lpt/callstack/call_stack.hpp>

class traced_exception : public std::exception
{
public:

    static const std::size_t max_stack_size = 20;
    typedef lpt::stack::call_stack<max_stack_size> stack_type;
    typedef lpt::stack::call_stack_info< stack_type
                                       , lpt::stack::extended_symbol_info    // extended_symbol_info, basic_symbol_info or null_symbol_info
                                       , lpt::stack::fancy_formatter         // fancy_ormatter or terse_formatter
                                       >     call_stack_info_type;

    traced_exception()
        : std::exception()
        , _where(true) // Capture stack
    {}

    const stack_type& where() const throw()
    {
        return _where;
    }

    const char * what() const throw()
    {
        return "traced_exception";
    }

private:

    stack_type _where;
};

void fun*c2()
{
    throw traced_exception();
}

void func1()
{
    func2();
}

int main()
{
    try
    {
        func1();
    }
    catch (const traced_exception& ex)
    {
        /*
            Exception: traced_exception
            Stack is 5 frames depth:
            [0x560b3f276a66] func2()+0x76
                    At /usr/include/c++/11/bits/move.h:205
                    In ./callstack1
            [0x560b3f276aef] main+0x27
                    At /home/amelinte/work/github/lpt-tools/src/callstack/examples/exception.cpp:43
                    In ./callstack1
            [0x7fc201e29d90] __libc_start_call_main+0x7fc201e29d90
                    At ./csu/../sysdeps/nptl/libc_start_call_main.h:58
                    In /lib/x86_64-linux-gnu/libc.so.6
            [0x7fc201e29e40] __libc_start_main+0x80
                    At ./csu/../csu/libc-start.c:128
                    In /lib/x86_64-linux-gnu/libc.so.6
            [0x560b3f276cc5] _start+0x25
                    At ??:0
                    In ./callstack1
        */
        std::cout << "Exception: " << ex.what() << "\n"
                  << "Stack is " << ex.where().depth() << " frames depth:\n"
                  << traced_exception::call_stack_info_type(ex.where())
                  << std::endl;
    }

    return 0;
}


