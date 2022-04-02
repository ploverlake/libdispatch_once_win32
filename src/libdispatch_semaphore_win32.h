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

/*
 * IMPORTANT: This header file describes INTERNAL interfaces to libdispatch
 * which are subject to change in future releases of Mac OS X. Any applications
 * relying on these interfaces WILL break.
 */

#ifndef LIBDISPATCH_SEMAPHORE_WIN32_H_
#define LIBDISPATCH_SEMAPHORE_WIN32_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Do not use these functions directly */
typedef uintptr_t _dispatch_thread_semaphore_t;

_dispatch_thread_semaphore_t _dispatch_get_thread_semaphore(void);
void _dispatch_put_thread_semaphore(_dispatch_thread_semaphore_t);
void _dispatch_thread_semaphore_wait(_dispatch_thread_semaphore_t);
void _dispatch_thread_semaphore_signal(_dispatch_thread_semaphore_t);

#ifdef __cplusplus
}
#endif

#endif /* LIBDISPATCH_SEMAPHORE_WIN32_H_ */
