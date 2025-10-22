/*
 *  $Id: $
 *
 *  Copyright 2025 Aurelian Melinte.
 *  Released under GPL 3.0 or later.
 *
 */

#ifndef INCLUDED_symbol_hpp_235fc3e6_8e07_4cb0_bfdf_bfcd8c3c4d4e
#define INCLUDED_symbol_hpp_235fc3e6_8e07_4cb0_bfdf_bfcd8c3c4d4e

#pragma once

#include <lpt/access.hpp>

#include <iostream>
#include <stacktrace>

namespace lpt {

/*
 * Poor man's symbol resolution piggybacked on std::stacktrace
 * Clang 21 has no stacktrace support
 */

// Overcome standard committee's ovesights

#if defined(__cpp_lib_stacktrace)
#  if defined(__GNUG__) && ! defined(__clang__)
struct stacktrace_entry_address { typedef std::stacktrace_entry::native_handle_type std::stacktrace_entry::* type; };
template struct lpt::private_member<stacktrace_entry_address, &std::stacktrace_entry::_M_pc>;
struct symbol : public std::stacktrace_entry
{
    symbol(void* addr) : std::stacktrace_entry()
    {
        *this.*member<stacktrace_entry_address>::value = reinterpret_cast<std::stacktrace_entry::native_handle_type>(addr);
    }
    
}; // symbol
#  elif defined(__clang__)
// no __cpp_lib_stacktrace
struct symbol 
{
    symbol(void* addr) {}

    friend std::ostream& operator<<(std::ostream& os, const symbol* ps)
    {
        return os;
    }
    friend std::ostream& operator<<(std::ostream& os, const symbol& s)
    {
        return os;
    }
}; // symbol
#  elif defined(_MSC_VER)
struct stacktrace_entry_address { typedef void* std::stacktrace_entry::*type; };
template struct private_member<stacktrace_entry_address, &std::stacktrace_entry::_Address>;
struct symbol : public std::stacktrace_entry
{
    symbol(void* addr) : std::stacktrace_entry()
    {
        *this.*member<stacktrace_entry_address>::value = reinterpret_cast<std::stacktrace_entry::native_handle_type>(addr);
    }
}; // symbol
#  else
#    error "Unknown compiler"
#  endif
#else
// no __cpp_lib_stacktrace
struct symbol 
{
    symbol(void* addr) {}

    friend std::ostream& operator<<(std::ostream& os, const symbol* ps)
    {
        return os;
    }
    friend std::ostream& operator<<(std::ostream& os, const symbol& s)
    {
        return os;
    }
}; // symbol
#endif // __cpp_lib_stacktrace


} //namespace lpt


#endif //#define INCLUDED_symbol_hpp_235fc3e6_8e07_4cb0_bfdf_bfcd8c3c4d4e
