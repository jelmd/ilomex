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

#ifndef ILOMEX_COMMON_H
#define ILOMEX_COMMON_H

#define ILOMEX_VERSION "1.0.0"
#define ILOMEX_AUTHOR "Jens Elkner (jel+ilomex@cs.uni-magdeburg.de)"

#include <stdbool.h>
#include <stdint.h>

#include <prom_string_builder.h>
#include <prom_log.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBUF_SZ 256

typedef struct node_cfg {
	bool no_node;
} node_cfg_t;

#define addPromInfo(metric) {\
	psb_add_char(sb, '\n');\
	psb_add_str(sb, "# HELP " metric ## _N " " metric ## _D );\
	psb_add_char(sb, '\n');\
	psb_add_str(sb, "# TYPE " metric ## _N " " metric ## _T);\
	psb_add_char(sb, '\n');\
}

#define ILOMEXM_VERS_D "Software version information."
#define ILOMEXM_VERS_T "gauge"
#define ILOMEXM_VERS_N "ilomex_version"

#define ILOMEXM_POWER_D "Current power usage of the node "
#define ILOMEXM_POWER_T "gauge"
#define ILOMEXM_POWER_N "ilomex_node"

/*
#define ILOMEXM_XXX_D "short description."
#define ILOMEXM_XXX_T "gauge"
#define ILOMEXM_XXX_N "ilomex_yyy"

 */

#ifdef __cplusplus
}
#endif

#endif // ILOMEX_COMMON_H
