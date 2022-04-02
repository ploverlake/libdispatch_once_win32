/**
 * Copyright (c) 2017, Russell. All rights reserved.
 */

#ifndef LIBDISPATCH_ONCE_WIN32_H_
#define LIBDISPATCH_ONCE_WIN32_H_

#ifdef __cplusplus
extern "C" {
#endif

void internal_once(void (*routine)(void));

#ifdef __cplusplus
}
#endif

#endif /* LIBDISPATCH_ONCE_WIN32_H_ */
