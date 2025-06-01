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

#ifdef __cplusplus
extern "C" {
#endif

void collect_node(psb_t *sb, bool compact);

#ifdef __cplusplus
}
#endif

#endif  // ILOMEX_PROM_SOL_H
