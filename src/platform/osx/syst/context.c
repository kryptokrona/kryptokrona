// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include <string.h>
#include "context.h"

void makecontext(uctx *ucp, void (*func)(void), intptr_t arg)
{
    long *sp;

    memset(&ucp->uc_mcontext, 0, sizeof ucp->uc_mcontext);

#if defined(__arm64__)
    /* AArch64: get/setmcontext (asm.s) treat uc_mcontext as a raw save area
     * addressed by byte offset from its start: x0 at #8, x30 at #248 and SP at
     * #256. setmcontext ret's to x30, so the entry point must go into the x30
     * slot, the first argument into the x0 slot and the new stack into the SP
     * slot. (The x86-64 mc_rip/mc_rsp fields below are meaningless on ARM and
     * writing them left x30 = 0, which made the first fiber switch jump to
     * 0x0.) */
    sp = (long *)ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size / sizeof(long);
    sp = (long *)((uintptr_t)sp & ~(uintptr_t)15); /* 16-byte align top of stack */
    {
        long *ctx = (long *)&ucp->uc_mcontext;
        ctx[8 / sizeof(long)] = (long)arg;    /* x0  -> first argument      */
        ctx[248 / sizeof(long)] = (long)func; /* x30 -> entry point (ret)   */
        ctx[256 / sizeof(long)] = (long)sp;   /* SP  -> top of fiber stack  */
    }
#else
    ucp->uc_mcontext.mc_rdi = (long)arg;
    sp = (long *)ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size / sizeof(long);
    sp -= 1;
    sp = (void *)((uintptr_t)sp - (uintptr_t)sp % 16); /* 16-align for OS X */
    *--sp = 0;                                         /* return address */
    ucp->uc_mcontext.mc_rip = (long)func;
    ucp->uc_mcontext.mc_rsp = (long)sp;
#endif
}

int swapcontext(uctx *oucp, const uctx *ucp)
{
    if (getcontext(oucp) == 0)
        setcontext(ucp);
    return 0;
}
