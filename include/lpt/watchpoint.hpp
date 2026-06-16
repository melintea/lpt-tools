/*
 *  $Id: $
 *
 *  Copyright 2026 Aurelian Melinte.
 *  Released under GPL 3.0 or later.
 *
 *  \brief Hardware breakpoints
 *  @see https://www.timdbg.com/posts/writing-a-debugger-from-scratch-part-5/
 *  @see https://aarzilli.github.io/debugger-bibliography/hwbreak.html
 *  @see https://github.com/mmorearty/hardware-breakpoints
 *  @see https://www.kernel.org/doc/ols/2009/ols2009-pages-149-158.pdf
 *
 */

#ifndef INCLUDED_watchpoint_hpp_7bc8cd35_dd8f_4b02_a5bb_74fb6490175c
#define INCLUDED_watchpoint_hpp_7bc8cd35_dd8f_4b02_a5bb_74fb6490175c

#pragma once

#include <stdio.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/user.h>
#include <math.h>
#include <sys/wait.h>
#include <inttypes.h>
#include <string.h>

namespace lpt { namespace intel {

#define DR_OFFSET(dr) ((((struct user *)0)->u_debugreg) + (dr))

// Used DR0 (out of DR0-3 available, DR7 debug control)
void watchpoint(void* addr)
{
    pid_t proc(getpid());
    struct user_regs_struct regs = {};
    
    ptrace(PTRACE_GETREGS, proc, NULL, &regs);
    ptrace(PTRACE_POKEUSER, proc, DR_OFFSET(0), addr);
	
    uint64_t dr7 = 1 + (1 << 8) + (1 << 9) + (1 << 10);
    ptrace(PTRACE_POKEUSER, proc, DR_OFFSET(7), dr7);
    ptrace(PTRACE_POKEUSER, proc, DR_OFFSET(6), 0);
    ptrace(PTRACE_CONT, proc, NULL, NULL);
}

}} //namespace lpt::intel

#endif //#define INCLUDED_watchpoint_hpp_7bc8cd35_dd8f_4b02_a5bb_74fb6490175c
