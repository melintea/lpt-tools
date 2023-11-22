/*
 *  $Id: $
 *
 *  Copyright 2023 Aurelian Melinte.
 *  Released under GPL 3.0 or later.
 *
 *  You need clang++.
 *
 */

#ifndef INCLUDED_llvm_mca_hpp_7bb266a6_f5ca_4e11_86f6_b5f0e940c5c5
#define INCLUDED_llvm_mca_hpp_7bb266a6_f5ca_4e11_86f6_b5f0e940c5c5

#pragma once

namespace lpt { namespace llvm {

/*
 * Machine Code Analyzer wrapper
 * https://llvm.org/docs/CommandGuide/llvm-mca.html
 */
class mca
{
public:

    mca()  { __asm volatile("# LLVM-MCA-BEGIN"); }
    ~mca() { __asm volatile("# LLVM-MCA-END");   }

    mca( const mca& other )            = delete;
    mca& operator=( const mca& other ) = delete;

    mca( const mca& other )            = delete;
    mca& operator=( mca&& other )      = delete;

}; // mca

}} //namespace lpt::llvm


#endif //#define INCLUDED_llvm_mca_hpp_7bb266a6_f5ca_4e11_86f6_b5f0e940c5c5
