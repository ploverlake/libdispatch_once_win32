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

#include "libdispatch_semaphore_win32.h"
#include <Windows.h>

static _dispatch_thread_semaphore_t _dispatch_thread_semaphore_create(void)
{
    HANDLE s4;
    s4 = CreateSemaphore(0, 0, 0x7FFFFFFF, 0);
    return (_dispatch_thread_semaphore_t)s4;
}

static void _dispatch_thread_semaphore_dispose(_dispatch_thread_semaphore_t sema)
{
    HANDLE s4 = (HANDLE)sema;
    BOOL ret = CloseHandle(s4);
}

void _dispatch_thread_semaphore_signal(_dispatch_thread_semaphore_t sema)
{
    HANDLE s4 = (HANDLE)sema;
    BOOL ret = ReleaseSemaphore(s4, 1, NULL);
}

void _dispatch_thread_semaphore_wait(_dispatch_thread_semaphore_t sema)
{
    HANDLE s4 = (HANDLE)sema;
    DWORD ret;
    do {
        ret = WaitForSingleObject(s4, INFINITE);
    } while (ret != WAIT_OBJECT_0);
}

_dispatch_thread_semaphore_t _dispatch_get_thread_semaphore(void)
{
    return _dispatch_thread_semaphore_create();
}

void _dispatch_put_thread_semaphore(_dispatch_thread_semaphore_t sema)
{
    _dispatch_thread_semaphore_dispose(sema);
}

