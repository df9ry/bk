#ifndef _MODULE_H
#define _MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "service.h"
#include "error.h"

struct module_t {
    bk_error_t (*load)  (const char      *id,
                         const service_t *sys_service,
                         const char      *meta);
    bk_error_t (*start) ();
    bk_error_t (*stop)  ();
};

#ifdef __cplusplus
}
#endif

#endif // _MODULE_H //
