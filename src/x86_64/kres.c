/**
 * @file kres.c
 * @author Kevin Oehme
 * @copyright MIT
 */

#include "kres.h"

/* EXTERNAL API */
// -

/* PUBLIC API */
kresult_t kresult_ok(const ch* msg);
kresult_t kresult_err(kstatus_t c, const ch* msg);

/* INTERNAL API */
// -

kresult_t kresult_ok(const ch* msg) { return (kresult_t){.code = K_OK, .msg = msg}; };

kresult_t kresult_err(const kstatus_t c, const ch* msg) { return (kresult_t){.code = c, .msg = msg}; };