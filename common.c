/*
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License") 1.1!
 * You may not use this file except in compliance with the License.
 *
 * See  https://spdx.org/licenses/CDDL-1.1.html  for the specific
 * language governing permissions and limitations under the License.
 *
 * Copyright 2025 Jens Elkner (jel+ilomex-src@cs.ovgu.de)
 */
#include <time.h>

#include "common.h"

#ifdef __linux
hrtime_t
gethrtime(void) {
	struct timespec ts;
	hrtime_t result;

#ifdef CLOCK_MONOTONIC_HR
	clock_gettime(CLOCK_MONOTONIC_HR, &ts);
#else
	clock_gettime(CLOCK_MONOTONIC, &ts);
#endif
	result = 1000000000LL * ts.tv_sec;
	result += ts.tv_nsec;
	return result;
}
#endif
