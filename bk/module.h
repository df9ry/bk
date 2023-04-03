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
    service_t  ifc; // Interface
    void      *ctx; // Context (self of service provider)
};

// Interface to publish and withdraw services:
struct admin_t {
    // Publish a service in the service registry:
    bk_error_t (*publish)  (const char*         module_id, // Module ID
                            const char*         name,      // Service name
                            const char*         meta,      // Service metadata
                            const service_reg_t service);  // Service registration
    // Withdraw a service from the service registry:
    bk_error_t (*withdraw) (const char*         module_id, // Module ID
                            const char*         name);     // Service name
    // Debug output to stdout / stderr:
    void       (*debug) (grade_t grade, const char* text);
    // Dump data block:
    void       (*dump)  (const char* text, const char* pb, size_t cb);
};

// Lookup interface:
struct lookup_t {
    const service_reg_t* (*find_service) (const char* name); // Lookup service
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
