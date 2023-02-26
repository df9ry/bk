#ifndef _SERVICE_H
#define _SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "error.h"

#include <stdint.h>
#include <stddef.h>

// Callback function for get and xch call:
typedef void (*response_f) (int         session_id,      // from open_session()
                            const char* response_meta,
                            const char* p_response_body,
                            size_t      c_response_body,
                            void*       user_data);

// Any session might offer this interface:
struct session_t {
    // Get data from server, no request data:
    bk_error_t (*get)(int         session_id,            // from open_session()
                      const char* request_meta,
                      response_f  response_function, void* user_data);

    // Put data to server, no response data:
    bk_error_t (*put)(int         session_id,            // from open_session()
                      const char* request_meta,
                      const char* p_request_body,
                      size_t      c_request_body);

    // Get data from server with request data:
    bk_error_t (*xch)(int         session_id,            // from open_session()
                      const char* request_meta,
                      const char* p_request_body,
                      size_t      c_request_body,
                      response_f  response_function, void* user_data);
};

struct service_t {
    bk_error_t (*open_session)  (const char* meta,
                                 session_t** remote_session_ifc_ptr,
                                 int*        remote_session_id_ptr);
    bk_error_t (*close_session) (int         remote_session_id);
};

#ifdef __cplusplus
}
#endif
#endif // _SERVICE_H //
