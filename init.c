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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <prom.h>

#include "common.h"
#include "init.h"

#include "prom_node.h"

static uint8_t started = 0;

static char *versionProm = NULL;	// version string emitted via /metrics
static char *versionHR = NULL;		// version string emitted to stdout/stderr


void
start(bool compact, uint32_t *tasks) {
	*tasks = 0;
	if (started)
		return;

	if (!compact)
		PROM_INFO("ilo stack initialized (%d)", *tasks);
	(*tasks)++;

	started = 1;
}

void
stop() {
	free(versionHR);
	versionHR = NULL;
	free(versionProm);
	versionProm = NULL;
	PROM_DEBUG("Node stack has been properly shutdown", "");
	started = 0;
}

char *
getVersions(psb_t *sbp, bool compact) {
	psb_t *sbi = NULL, *sb = NULL;

	if (versionProm != NULL) {
		goto end;
	}

	sbi = psb_new();
	sb = psb_new();
	if (sbi == NULL || sb == NULL) {
		psb_destroy(sbi);
		psb_destroy(sb);
		return NULL;
	}

	psb_add_str(sbi, "ilomex " ILOMEX_VERSION "\n(C) 2025 " ILOMEX_AUTHOR "\n");
	versionHR = psb_dump(sbi);
	psb_destroy(sbi);

	if (!compact)
		addPromInfo(ILOMEXM_VERS);

	psb_add_str(sb, ILOMEXM_VERS_N "{name=\"server\",value=\"" ILOMEX_VERSION
		"\"} 1\n");

	versionProm = psb_dump(sb);
	psb_destroy(sb);

end:
	if (sbp == NULL) {
		fprintf(stdout, "%s", versionHR);
	} else {
		psb_add_str(sbp, versionProm);
	}
	return versionHR;
}
