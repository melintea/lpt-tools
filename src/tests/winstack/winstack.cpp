//

#include "stdafx.h"

#include <lpt/call_stack.hpp>

#include <iostream>


typedef lpt::stack::call_stack<40> wstack_type;
typedef lpt::stack::call_stack_info< wstack_type
                                   , lpt::stack::null_symbol_info
                                   , lpt::stack::terse_formatter
                                   >     null_call_stack_info_type;
typedef lpt::stack::call_stack_info< wstack_type
                                   , lpt::stack::basic_symbol_info
                                   >     basic_call_stack_info_type;
typedef lpt::stack::call_stack_info< wstack_type
                                   , lpt::stack::extended_symbol_info
                                   >     extended_call_stack_info_type;


int func2(int p1, const char* p2)
{
    std::cout << __FUNCTION__  << std::endl;

    wstack_type here(true);
    assert( here.depth() );

    std::cout << "**\n[ null_symbol_info stack: " << here.depth() << "\n"
              << null_call_stack_info_type(here) << "**] Stack\n\n" << std::flush;
    std::cout << "**\n[ basic_symbol_info stack: " << here.depth() << "\n"
              << basic_call_stack_info_type(here) << "**] Stack\n\n" << std::flush;
    std::cout << "**\n[ extended_symbol_info stack: " << here.depth() << "\n"
              << extended_call_stack_info_type(here) << "**] Stack\n\n" << std::flush;


    return p1;
}


class aclass
{
public:
    void meth2(int p1, const char* p2)
    {
        std::cout << __FUNCTION__  << std::endl;
        func2(p1, p2);
    }

    void meth1(int p1)
    {
        std::cout << __FUNCTION__  << std::endl;
        meth2(p1, "meth1");
    }

};


int func1(int p1)
{
    std::cout << __FUNCTION__ << std::endl;

    aclass x;
    x.meth1(p1);

    return p1;
}


int _tmain(int argc, _TCHAR* argv[])
{
    std::cout << "Compiler version: " << WINVER << std::endl;
    func1(1);

    //
    wstack_type s1(true);
    wstack_type s2;
    std:: cout << "\n*** s2 must be empty:\n " << extended_call_stack_info_type(s2);
    std:: cout << "\n*** s1:\n" << extended_call_stack_info_type(s1) << std::flush;
    assert( s2.depth() == 0 && s1.depth() > 0 );
    std::swap(s1, s2);
    std:: cout << "\n*** s1 must be empty:\n " << extended_call_stack_info_type(s1);
    std:: cout << "\n*** s2:\n" << extended_call_stack_info_type(s2) << std::flush;
    assert( s1.depth() == 0 && s2.depth() > 0 );

    //
    extended_call_stack_info_type i1(s1);
    extended_call_stack_info_type i2(s2);
    std::swap(i1, i2);
    assert( s1.depth() == 0 && s2.depth() > 0 );

    //
    lpt::stack::extended_symbol_info ei1(0x1111);
    lpt::stack::extended_symbol_info ei2(0x2222);
    std::swap(ei1, ei2);
    assert( s1.depth() == 0 && s2.depth() > 0 );

    lpt::stack::call_frame f1(0x1111), f2;
    lpt::stack::call_frame_info< lpt::stack::extended_symbol_info
                               , lpt::stack::fancy_formatter > fi1(f1), fi2(f2);
    assert(f1.addr() != lpt::stack::null_address_type && f2.addr() == lpt::stack::null_address_type);
    std::swap(f1, f2);
    assert(f2.addr() != lpt::stack::null_address_type && f1.addr() == lpt::stack::null_address_type);

    assert(fi1.frame().addr() != lpt::stack::null_address_type && fi2.frame().addr() == lpt::stack::null_address_type);
    std::swap(fi1, fi2);
    assert(fi2.frame().addr() != lpt::stack::null_address_type && fi1.frame().addr() == lpt::stack::null_address_type);

    lpt::stack::call_stack<1> one(true);
    assert( one.depth() == 1 );
    std::cout << "\n*** Stack with depth one: \n" 
              << "depth=" << one.depth() << "\n" 
              << lpt::stack::call_stack_info< lpt::stack::call_stack<1>
                                            , lpt::stack::extended_symbol_info >(one)
              ;

    for (size_t i=0; i< s2.size() ; ++i) {
        lpt::stack::call_frame aframe( s2[i] );
        std::cout << "\n*** Frame: " << i << "\n" 
                  << lpt::stack::call_frame_info< lpt::stack::extended_symbol_info
                                                , lpt::stack::fancy_formatter >(aframe) << "\n"
                  ;
    }

	return 0;
}

