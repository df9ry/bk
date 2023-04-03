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
    bk_error_t (*get) (void* session_ctx, const char* head, resp_f fun, void* ctx);
    bk_error_t (*post)(void* session_ctx, const char* head, const char* p_body, size_t c_body);
};

// Type for session registration:
struct session_reg_t {
    session_t  ifc; // Interface
    void      *ctx; // Context (self of service provider)
};

// Any service might offer this interface:
struct service_t {
    bk_error_t (*open_session)  (void* server_ctx, const char* meta, session_reg_t* reg);
    bk_error_t (*close_session) (void* server_ctx, const void* session_ctx);
};

#ifdef __cplusplus
}
#endif

#endif // _SERVICE_H //
