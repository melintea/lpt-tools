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

void func2()
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
        std::cout << "Exception: " << ex.what() << "\n"
                  << "Stack is " << ex.where().depth() << " frames depth:\n"
                  << traced_exception::call_stack_info_type(ex.where())
                  << std::endl;
    }

    return 0;
}


