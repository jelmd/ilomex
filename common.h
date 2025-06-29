/**
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

#ifdef __linux
typedef uint64_t hrtime_t;
hrtime_t gethrtime(void);
#endif

#define NS_SECOND 1000 * 1000 * 1000
#define NS_MINUTE 60L * NS_SECOND

#define MBUF_SZ 256

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

#define ILOMEXM_POWER_D "Power usage of the node in Watt"
#define ILOMEXM_POWER_T "gauge"
#define ILOMEXM_POWER_N "ilomex_node_power_W"

#define ILOMEXM_PSU_D "Number of Power Supply Units of the node"
#define ILOMEXM_PSU_T "gauge"
#define ILOMEXM_PSU_N "ilomex_node_psu"

/*
#define ILOMEXM_XXX_D "short description."
#define ILOMEXM_XXX_T "gauge"
#define ILOMEXM_XXX_N "ilomex_yyy"

 */

#ifdef __cplusplus
}
#endif

#endif // ILOMEX_COMMON_H
