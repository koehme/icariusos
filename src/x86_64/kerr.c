/**
 * @file kerr.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "kerr.h"

/* EXTERNAL API */
// -

/* PUBLIC API */
kresult_t kres_ok(const char* msg);
kresult_t kres_err(kstatus_t c, const char* msg);

/* INTERNAL API */
// -

kresult_t kres_ok(const char* msg) { return (kresult_t){.code = K_OK, .msg = msg}; };

kresult_t kres_err(kstatus_t c, const char* msg) { return (kresult_t){.code = c, .msg = msg}; };