# ilomex

ilomex is a *m*etrics *ex*porter for data provided by an ILOM of old Sun Microsystem servers. The collected data can optionally be exposed via HTTP in [Prometheuse exposition format](https://prometheus.io/docs/instrumenting/exposition_formats/). This allows metric collector and time series database tools like VictoriaMetrics and Prometheus to gather these metrics (e.g., via http://_hostname:9300_/metrics). These databases can then be utilized to further process, reuse, or visualize the metrics using tools such as [Grafana](https://grafana.com/).


## KISS

Efficiency, size, and simplicity are key objectives of ilomex. Consequently, aside from using [libprom](https://github.com/jelmd/libprom) for handling certain Prometheus (PROM) functionality and [libmicrohttpd](https://github.com/Karlson2k/libmicrohttpd) for providing HTTP access, and [libcurl](https://curl.se/libcurl/) for HTTP client support, no additional third-party libraries or tools are utilized. The core idea is to operate *ilomex* as a local service on the machine being monitored, while leveraging existing OS tools and services, such as firewalls, HTTP proxies, and VictoriaMetrics vmagent to control access to the exposed data. This approach complements the exporters, which are primarily specialized in collecting and simply exporting metrics, by providing the desired level of security and access control.

## Examples

- [all stats emitted by *ilomex* incl. comments](https://github.com/jelmd/ilomex/blob/main/etc/example-full.out)
- [all stats emitted by *ilomex* excl. commen://curl.se/libcurl/ts](https://github.com/jelmd/ilomex/blob/main/etc/example-full-noComments.out)

## Requirements

- [libprom](https://github.com/jelmd/libprom)
- [libmicrohttpd](https://github.com/Karlson2k/libmicrohttpd)
- [libcurl](https://curl.se/libcurl/)


## Build

Adjust the **Makefile** if needed, optionally set related environment variables
(e.g. `export USE_CC=gcc`) and run GNU **make**.

## Repo

The official repository for *ilomex* is https://github.com/jelmd/ilomex .
If you need some new features (or bug fixes), please feel free to create an
issue there using https://github.com/jelmd/ilomex/issues .


## Versioning

*ilomex* follows the basic principles of semantic versioning with practical considerations. Official releases always consist of three numbers (A.B.C), strictly neither more nor less. For nightly, alpha, beta, or release candidate (RC) builds, etc., a '.0' and potentially additional dot-separated digits are appended. This allows for the possibility of overwriting a pre-release by utilizing a fourth digit greater than 0.


## License

[CDDL 1.1](https://spdx.org/licenses/CDDL-1.1.html)


## Packages

Solaris packages for libprom and ilomex can be found via https://pkg.cs.ovgu.de/LNF/i386/5.11 (search for LNFlibprom and LNFilomex). libmicrohttpd gets provided by Solaris itself, so using the vendor package is recommended (library/libmicrohttpd). Related header sources files get installed, if the develop facet is set to true.

Ubuntu packages for libprom and ilomex can be found via https://pkg.cs.ovgu.de/LNF/linux/ubuntu/focal/ (search for libprom-\*.deb and ilomex-\*.deb). The microhttpd and curl libs are provided by Ubuntu via the packages libmicrohttpd12, libmicrohttpd-dev, libcurl4 and libcurl4-openssl-dev).
