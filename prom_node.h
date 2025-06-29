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

/**
 * @file prom_node.h
 * Prom related defintions/functions/etc.
 */
#ifndef ILOMEX_PROM_SOL_H
#define ILOMEX_PROM_SOL_H

#include <curl/curl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum target_flag {
	TARGET_HTTP = 1,
	TARGET_HTTPS = 2,
	TARGET_INSECURE = 4,
	TARGET_WRONGPW = 8,
	TARGET_INVALID = 16
} target_flag_t;

typedef enum power_data {
	PWR_ACTUAL = 0,
	PWR_PERMITTED,
	PWR_AVAIL,
	PWR_PSUS,
	PWR_PSUS_RESERVED,
	PWR_DATA_MAX
} power_data_t;

typedef struct target {
	char *host;
	char *login;
	char *pw;
	uint32_t flags;
	uint32_t port;
	CURL *hdl;
	int timeout_s;
	hrtime_t last_check;	// when the last check was scheduled
	time_t timestamp;		// the time the power[] values have been rceived
	char *url;
	char *body;
	size_t body_sz;
	size_t body_capacity;
	bool verbose;
	hrtime_t skip_til;
	char *token;
	int power[PWR_DATA_MAX];
	struct target *next;
} target_t;

void collect_power(psb_t *sb, bool compact, target_t *ilom, hrtime_t now);

#ifdef __cplusplus
}
#endif

#endif  // ILOMEX_PROM_SOL_H
