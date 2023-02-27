#ifndef _SERVICE_H
#define _SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "error.h"

#include <stdint.h>
#include <stddef.h>

typedef void (*resp_f)(void* client_ctx, const char* head, const char* p_body, size_t c_body);

// Any session might offer this interface:
struct session_t {
    bk_error_t (*get) (void* server_ctx, const char* head, resp_f fun);
    bk_error_t (*post)(void* server_ctx, const char* head, const char* p_body, size_t c_body);
};

// Any service might offer this interface:
struct service_t {
    bk_error_t (*open_session)
        (void* client_loc_ctx, void** server_ctx_ptr, const char* meta, session_t** ifc_ptr);
    bk_error_t (*close_session) (void* server_ctx);
};

#ifdef __cplusplus
}
#endif

#endif // _SERVICE_H //
