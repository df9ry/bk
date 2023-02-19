#ifndef _MODULE_H
#define _MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "service.h"

struct module_t {
    void (*load)  (const char *id, 
                   const service_t *sys_service,
                   const char *meta);
    void (*start) ();
    void (*stop)  ();
};

#ifdef __cplusplus
}
#endif

#endif // _MODULE_H //