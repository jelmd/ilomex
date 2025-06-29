/*
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License") 1.1!
 * You may not use this file except in compliance with the License.
 *
 * See  https://spdx.org/licenses/CDDL-1.1.html  for the specific
 * language governing permissions and limitations under the License.
 *
 * Copyright 2021 Jens Elkner (jel+ilomex-src@cs.ovgu.de)
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <curl/curl.h>

#include <prom_string_builder.h>

#include "common.h"
#include "prom_node.h"

#define LOGIN_PAGE "/iPages/loginProcessor.asp"
#define POWER_PAGE "/iPages/i_powermgmt.asp"

#define MAX_URL_LEN 1024

size_t
read_response_cb(void *contents, size_t size, size_t nmemb, void *userp) {
	target_t *ilom = userp;
    size_t sz = size * nmemb;
	size_t need_sz = ilom->body_sz + sz + 1;
	if (need_sz > ilom->body_capacity) {
		char *nb = realloc(ilom->body, need_sz);
		if (nb == NULL) {
			PROM_ERROR("Not enough memory to store ilom response for %s", ilom->host);
			return 0;
		}
		ilom->body = nb;
		ilom->body_capacity = need_sz;
	}
	memcpy(&(ilom->body[ilom->body_sz]), contents, sz);
	ilom->body_sz += sz;
	ilom->body[ilom->body_sz] = '\0';
    return sz;
}

static int
check_ilom(target_t *ilom) {
	char buf[MAX_URL_LEN];
	char dbuf[8];

	if(!ilom->hdl)
		ilom->hdl = curl_easy_init();
	if (!ilom->url) {
		if (ilom->flags & TARGET_HTTP) {
			strcpy(buf, "http://");
			ilom->flags = ilom->flags & TARGET_HTTP;
		} else {
			strcpy(buf, "https://");
			ilom->flags = ilom->flags & TARGET_HTTPS
				? TARGET_HTTPS
				: TARGET_INSECURE;
		}
		if (!ilom->host)
			return 1;
		strcat(buf, ilom->host);
		if (ilom->port) {
			if (ilom->port > 65535) {
				PROM_ERROR("Invalid port for %s (skipping)", ilom->host);
				ilom->flags |= TARGET_INVALID;
			}
			sprintf(dbuf, ":%d", ilom->port);
			strcat(buf, dbuf);
		}
		if (strlen(buf) + strlen(LOGIN_PAGE) + 2 > MAX_URL_LEN) {
			PROM_ERROR("URL too long for %s (skipping)", ilom->host);
			ilom->flags |= TARGET_INVALID;
		}
		if (ilom->url)
			free(ilom->url);
		ilom->url = (ilom->flags & TARGET_INVALID) ? NULL : strdup(buf);
	}
	curl_easy_setopt(ilom->hdl, CURLOPT_IPRESOLVE, CURLOPT_DNS_LOCAL_IP4);
	curl_easy_setopt(ilom->hdl, CURLOPT_WRITEFUNCTION, read_response_cb);
	curl_easy_setopt(ilom->hdl, CURLOPT_WRITEDATA, ilom);
	curl_easy_setopt(ilom->hdl, CURLOPT_FOLLOWLOCATION, 0);
	curl_easy_setopt(ilom->hdl, CURLOPT_USERAGENT, "ilomex/" VERSION);
	curl_easy_setopt(ilom->hdl, CURLOPT_VERBOSE, ilom->verbose ? 1 : 0);
	curl_easy_setopt(ilom->hdl, CURLOPT_TIMEOUT, ilom->timeout_s);

	/* ILOM is HTTP/1.0 and does not support keep-alive
	curl_easy_setopt(ilom->hdl, CURLOPT_TCP_KEEPALIVE, 1L);
	curl_easy_setopt(ilom->hdl, CURLOPT_TCP_KEEPINTVL, 60L);	// ping once a minute
	curl_easy_setopt(ilom->hdl, CURLOPT_TCP_KEEPIDLE, 600L);	// for max. 10 min
	*/
	return ilom->flags & TARGET_INVALID ? 1 : 0;
}

#define MAX_REDIRECTS 5

typedef struct power_metric_meta {
	const char *lookup;
	size_t lookup_len;
	const char *mname;
	const char *mattr;
	const char *mtype;
	const char *mdesc;
} power_metric_t;

#define LOOKUP_AND_SZ(x, y, z) \
	{ x , sizeof(x) - 1, ILOMEXM_ ## y ## _N , z, ILOMEXM_ ## y ## _T, ILOMEXM_ ## y ## _D }

// To find the start of the number to extract we need to lookup the following
// strings. Since the occure always in the same order, we can optimize it
// accordingly.
static power_metric_t PWR_METRIC[PWR_DATA_MAX] = {
	LOOKUP_AND_SZ("var actual_power = '", POWER, "actual"),
	LOOKUP_AND_SZ("var permitted_power = '", POWER, "permitted"),
	LOOKUP_AND_SZ("var available_power = '", POWER, "avail"),
	LOOKUP_AND_SZ("var total_psus = '", PSU, "total"),
	LOOKUP_AND_SZ("var reserve_psus = '", PSU, "reserved")
};

static int
fetch_power_data(target_t *ilom, const char *path) {
	char buf[MAX_URL_LEN];
	char post[MAX_URL_LEN];
	CURLcode res;
	long code;
	int i;
	char *location, *end;
	bool need_login = false;
	uint8_t redirects = 0;

	if (ilom->verbose)
		PROM_INFO("Fetching %s%s", ilom->url, path);
	strcpy(buf, ilom->url);
	strcat(buf, path);
	curl_easy_setopt(ilom->hdl, CURLOPT_POST, 0);
again:
	ilom->body_sz = 0;
	curl_easy_setopt(ilom->hdl, CURLOPT_URL, buf);
	if (ilom->token) {
		snprintf(buf, MAX_URL_LEN, "ilom=1; WebSessionString_SP=%s; langsetting=EN; "
			"ActivityHappened_SP=1; ActivityHappened=1", ilom->token);
		curl_easy_setopt(ilom->hdl, CURLOPT_COOKIE, buf);
	} else {
		curl_easy_setopt(ilom->hdl, CURLOPT_COOKIE, "");
	}
	res = curl_easy_perform(ilom->hdl);
	if (res != CURLE_OK) {
		PROM_WARN("Failed to fetch %s: %s", ilom->host, curl_easy_strerror(res));
		return 0;
	}
	//fprintf(stderr, "#####################\n%s\n\n", ilom->body);
	curl_easy_getinfo(ilom->hdl, CURLINFO_RESPONSE_CODE, &code);
	if (code == 303) {
		curl_easy_getinfo(ilom->hdl, CURLINFO_REDIRECT_URL, &location);
		if (redirects > MAX_REDIRECTS || location == NULL)
			return 0;
		redirects++;
		if (strcmp(location + strlen(location) - 12, "/timeout.asp") == 0) {
			need_login = true;
			strcpy(buf, ilom->url);
			strcat(buf, LOGIN_PAGE);
			snprintf(post, sizeof(post),
				"sclink=&username=%s&password=%s&button=Log+In",
				ilom->login, ilom->pw);
			curl_easy_setopt(ilom->hdl, CURLOPT_POST, 1L);
			curl_easy_setopt(ilom->hdl, CURLOPT_POSTFIELDS, post);
			free(ilom->token);
			ilom->token = NULL;
		} else {
			snprintf(buf, MAX_URL_LEN, "%s", location);
		}
		PROM_DEBUG("Redirecting to %s\n", buf);
		goto again;
	}
	if (code != 200) {
		curl_easy_getinfo(ilom->hdl, CURLINFO_EFFECTIVE_URL, &location);
		PROM_WARN("%s returned HTTP status code %d", location, code);
		return 0;
	}
	if (need_login) {
		if (((location = strchr(ilom->body, '"')) == NULL)
			|| (strncmp(location, "\"WebSessionString_SP\",\"", 23) != 0))
		{
			PROM_WARN("Failed to login to '%s'.", ilom->host);
			return 0;
		}
		location += 23;
		if ((end = strchr(location, '"')) == NULL) {
			PROM_WARN("Unable to extract session token for '%s'-", ilom->host);
			return 0;
		}
		free(ilom->token);
		ilom->token = strndup(location, end - location);
		PROM_DEBUG("Obtained token '%s'", ilom->token);
		strcpy(buf, ilom->url);
		strcat(buf, path);
		need_login = false;
		goto again;
	}
	// finally we should have the power readings. So look them up:
	// Skip the 1st ~ 1175 chars
	if ((location = strstr(ilom->body, PWR_METRIC[0].lookup)) == NULL) {
		curl_easy_getinfo(ilom->hdl, CURLINFO_EFFECTIVE_URL, &location);
		PROM_WARN("Unable to extract power readings from '%s'.", ilom->host);
		return 0;
	}
	// Actually all value should be in a block of ~ 130..140 chars, but to
	// make it a little bit more stable we double it and stop than parsing the
	// remaining ~7 KiB of junk.
	location[255] = '\0';
	code = ilom->body_sz;
	ilom->body_sz = location - ilom->body + 256;

	#define MARK "######################################################"
	//PROM_DEBUG(MARK "\n%ld/%ld\n%s\n" MARK "\n", ilom->body_sz, code, location);

	errno = 0;
	location += PWR_METRIC[0].lookup_len;
	code = strtol(location, &end, 10);
	if (errno || end == location || *end != '\'') {
		curl_easy_getinfo(ilom->hdl, CURLINFO_EFFECTIVE_URL, &location);
		PROM_WARN("Unable to extract actual power reading from '%s'.", location);
		return 0;
	}
	ilom->power[PWR_ACTUAL] = code;

	redirects = 0;
	for (i=1; i < PWR_DATA_MAX; i++) {
not_found:
		if ((location = strstr(end, PWR_METRIC[i].lookup)) == NULL) {
			ilom->power[i] = -1;	// mark n/a
			continue;
		}
		location += PWR_METRIC[i].lookup_len;
		errno = 0;
		code = strtol(location, &end, 10);
		if (errno || end == location || *end != '\'') {
			end = location + 1;
			ilom->power[i] = -1;	// mark n/a
			goto not_found;
		}
		ilom->power[i] = code;
		redirects++;
	}
	time(&(ilom->timestamp));
	// for (i=0; i < PWR_DATA_MAX; i++) {
	// 	PROM_INFO("%s %s = %d", PWR_METRIC[i].mname, PWR_METRIC[i].mattr, ilom->power[i]);
	// }
	return redirects;
}

// NOTE: ILOM allows ~10 parallel sessions, only. If they are idle for at least
//       ~15 min (30, 60 and 180 min are also possible) they get terminated
//		automatically. See also  System Information | Session Time-Out
#define PAUSE 5

void
collect_power(psb_t *sb, bool compact, target_t *ilom, hrtime_t now) {
	char buf[256];
	size_t sz = 0;
	bool free_sb = sb == NULL;
	int i;
	target_t *tbd = ilom;

	if (ilom == NULL || ilom->flags & TARGET_INVALID)
		return;

	if (check_ilom(ilom) != 0)
		return;

	if (free_sb) {
		sb = psb_new();
		if (sb == NULL) {
			perror("collect_power: ");
			return;
		}
		sz = psb_len(sb);
	}

	for (;tbd ; tbd = tbd->next) {
		if (check_ilom(tbd))
			continue;
		if (tbd->skip_til > now)
			continue;
		if (tbd->last_check + NS_SECOND <= now) {
			tbd->last_check = now;
			memset(tbd->power, -1, sizeof(tbd->power));
			if (fetch_power_data(tbd, POWER_PAGE) == 0) {
				PROM_INFO("Collection paused for %s (%d minutes).",
					tbd->host, PAUSE);
				ilom->skip_til = now + PAUSE * NS_MINUTE;
				continue;
			}
		}
		for (i=0; i < PWR_DATA_MAX; i++) {
			if (tbd->power[i] < 0)
				continue;
			if (!compact) {
				psb_add_char(sb, '\n');
				psb_add_str(sb, "# HELP ");
				psb_add_str(sb, PWR_METRIC[i].mdesc);
				psb_add_char(sb, '\n');
				psb_add_str(sb, "# TYPE ");
				psb_add_str(sb, PWR_METRIC[i].mtype );
				psb_add_char(sb, '\n');
			}
			psb_add_str(sb, PWR_METRIC[i].mname);
			psb_add_str(sb, "{val=\"");
			psb_add_str(sb, PWR_METRIC[i].mattr);
			psb_add_str(sb, "\",ilom=\"");
			psb_add_str(sb, tbd->host);
			sprintf(buf, "\"} %d %ld\n", tbd->power[i], tbd->timestamp);
			psb_add_str(sb, buf);
		}
	}

	if (free_sb) {
		if (psb_len(sb) != sz)
			fprintf(stdout, "\n%s", psb_str(sb));
		psb_destroy(sb);
	}
}
