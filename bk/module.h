#ifndef _MODULE_HPP
#define _MODULE_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include "service.h"

struct module_t {
    void (*load)  (const char *id, const service_t *sys_service);
    void (*start) (const char *meta);
    void (*stop)  ();
};

#ifdef __cplusplus
}
#endif

#endif // _MODULE_HPP //