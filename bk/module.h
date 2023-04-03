#ifndef _MODULE_H
#define _MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "service.h"
#include "error.h"

// Severity of a message:
enum grade_t {
    BK_DEBUG = 'd', BK_INFO = 'i', BK_WARNING = 'w', BK_ERROR = 'e', BK_FATAL = 'f'
};

// Type for service registration:
struct service_reg_t {
    void            *service_ctx; // Context (self of service provider)
    const service_t *service_ifc; // Interface
};

// Interface to publish and withdraw services:
struct admin_t {
    bk_error_t (*publish)  (const char* module_id, const char* meta,
                            const service_reg_t* service);
    bk_error_t (*withdraw) (const char* module_id, const char* name);
    void       (*debug) (grade_t grade, const char* text);
    void       (*dump)  (const char* text, const char* pb, size_t cb);
};

// Lookup interface:
struct lookup_t {
    const service_reg_t* (*find_service) (const char* server);
};

// Interface to handle loadable modules:
struct module_t {
    bk_error_t (*load)  (const char     *id,
                         const admin_t  *admin_ifc,
                         const char     *meta);
    bk_error_t (*start) (const lookup_t *lookup);
    bk_error_t (*stop)  ();
};

#ifdef __cplusplus
}
#endif

#endif // _MODULE_H //
