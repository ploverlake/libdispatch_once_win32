/**
 *  Copyright (c) 2017, Russell
 *  All rights reserved.
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  
 *  * Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *  
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Portions of this file are derived from libdispatch
 */

/*
 * Copyright (c) 2008-2011 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#include "libdispatch_once_win32.h"
#include <Windows.h>
#include <stdint.h>
#include "libdispatch_semaphore_win32.h"

/* Actually, _mm_pause and _mm_mfence are declared in <intrin.h> */
extern void _mm_pause(void);
#pragma intrinsic(_mm_pause)
#define _dispatch_hardware_pause _mm_pause

/**
 *  It is better to use mfence instruction than cpuid in fact,
 *  but unfortunately mfence was introduced from Intel Pentium 4, 
 *  so that running mfence on earlier CPU, for instance Pentium 3,
 *  will cause crash.
 */
#define dispatch_atomic_maximally_synchronizing_barrier() \
    do { \
        __asm \
        { \
            __asm mov eax, 0 \
            __asm cpuid \
        } \
    } while (0)

#define dispatch_atomic_acquire_barrier()
#define dispatch_atomic_store_barrier()

#define dispatch_atomic_xchg(_p, _n)         ((struct _dispatch_once_waiter_s *)InterlockedExchange((LONG volatile *)(_p), (LONG)(_n)))
#define dispatch_atomic_cmpxchg(_p, _o, _n)  (InterlockedCompareExchangePointer((PVOID volatile *)(_p), (_n), (_o)) == (_o))

#define _dispatch_client_callout(_ctx, _f) (_f)()
#define DISPATCH_ONCE_DONE ((struct _dispatch_once_waiter_s *)~0l)

typedef uintptr_t dispatch_once_t;
typedef void (*dispatch_function_t)(void);

struct _dispatch_once_waiter_s {
    volatile struct _dispatch_once_waiter_s *volatile dow_next;
    _dispatch_thread_semaphore_t dow_sema;
};

static __forceinline
void
dispatch_once_f(dispatch_once_t *val, void *ctxt, dispatch_function_t func)
{
    struct _dispatch_once_waiter_s * volatile *vval =
        (struct _dispatch_once_waiter_s**)val;
    struct _dispatch_once_waiter_s dow = { NULL, 0 };
    struct _dispatch_once_waiter_s *tail, *tmp;
    _dispatch_thread_semaphore_t sema;

    if (dispatch_atomic_cmpxchg(vval, NULL, &dow)) {
        dispatch_atomic_acquire_barrier();
        _dispatch_client_callout(ctxt, func);

        // The next barrier must be long and strong.
        //
        // The scenario: SMP systems with weakly ordered memory models
        // and aggressive out-of-order instruction execution.
        //
        // The problem:
        //
        // The dispatch_once*() wrapper macro causes the callee's
        // instruction stream to look like this (pseudo-RISC):
        //
        //      load r5, pred-addr
        //      cmpi r5, -1
        //      beq  1f
        //      call dispatch_once*()
        //      1f:
        //      load r6, data-addr
        //
        // May be re-ordered like so:
        //
        //      load r6, data-addr
        //      load r5, pred-addr
        //      cmpi r5, -1
        //      beq  1f
        //      call dispatch_once*()
        //      1f:
        //
        // Normally, a barrier on the read side is used to workaround
        // the weakly ordered memory model. But barriers are expensive
        // and we only need to synchronize once! After func(ctxt)
        // completes, the predicate will be marked as "done" and the
        // branch predictor will correctly skip the call to
        // dispatch_once*().
        //
        // A far faster alternative solution: Defeat the speculative
        // read-ahead of peer CPUs.
        //
        // Modern architectures will throw away speculative results
        // once a branch mis-prediction occurs. Therefore, if we can
        // ensure that the predicate is not marked as being complete
        // until long after the last store by func(ctxt), then we have
        // defeated the read-ahead of peer CPUs.
        //
        // In other words, the last "store" by func(ctxt) must complete
        // and then N cycles must elapse before ~0l is stored to *val.
        // The value of N is whatever is sufficient to defeat the
        // read-ahead mechanism of peer CPUs.
        //
        // On some CPUs, the most fully synchronizing instruction might
        // need to be issued.

        dispatch_atomic_maximally_synchronizing_barrier();
        //dispatch_atomic_release_barrier(); // assumed contained in above
        tmp = dispatch_atomic_xchg(vval, DISPATCH_ONCE_DONE);
        tail = &dow;
        while (tail != tmp) {
            while (!tmp->dow_next) {
                _dispatch_hardware_pause();
            }
            sema = tmp->dow_sema;
            tmp = (struct _dispatch_once_waiter_s*)tmp->dow_next;
            _dispatch_thread_semaphore_signal(sema);
        }
    } else {
        dow.dow_sema = _dispatch_get_thread_semaphore();
        for (;;) {
            tmp = *vval;
            if (tmp == DISPATCH_ONCE_DONE) {
                break;
            }
            dispatch_atomic_store_barrier();
            if (dispatch_atomic_cmpxchg(vval, tmp, &dow)) {
                dow.dow_next = tmp;
                _dispatch_thread_semaphore_wait(dow.dow_sema);
            }
        }
        _dispatch_put_thread_semaphore(dow.dow_sema);
    }
}

void internal_once(void (*routine)(void))
{
    static dispatch_once_t once_token = 0;
    if (once_token != (dispatch_once_t)DISPATCH_ONCE_DONE) {
        dispatch_once_f(&once_token, NULL, routine);
    }
}

