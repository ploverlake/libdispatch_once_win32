/**
 * Copyright (c) 2017, Russell. All rights reserved.
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

static _dispatch_thread_semaphore_t _dispatch_thread_semaphore_create(void) {
  HANDLE s4;
  s4 = CreateSemaphore(0, 0, 0x7FFFFFFF, 0);
  return (_dispatch_thread_semaphore_t)s4;
}

static void _dispatch_thread_semaphore_dispose(
    _dispatch_thread_semaphore_t sema) {
  HANDLE s4 = (HANDLE)sema;
  BOOL ret = CloseHandle(s4);
}

void _dispatch_thread_semaphore_signal(_dispatch_thread_semaphore_t sema) {
  HANDLE s4 = (HANDLE)sema;
  BOOL ret = ReleaseSemaphore(s4, 1, NULL);
}

void _dispatch_thread_semaphore_wait(_dispatch_thread_semaphore_t sema) {
  HANDLE s4 = (HANDLE)sema;
  DWORD ret;
  do {
    ret = WaitForSingleObject(s4, INFINITE);
  } while (ret != WAIT_OBJECT_0);
}

_dispatch_thread_semaphore_t _dispatch_get_thread_semaphore(void) {
  return _dispatch_thread_semaphore_create();
}

void _dispatch_put_thread_semaphore(_dispatch_thread_semaphore_t sema) {
  _dispatch_thread_semaphore_dispose(sema);
}
