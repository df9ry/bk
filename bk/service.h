#ifndef _SERVICE_H
#define _SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "error.h"
#include "session.h"

enum grade_t {
    BK_DEBUG = 'd', BK_INFO = 'i', BK_WARNING = 'w', BK_ERROR = 'e', BK_FATAL = 'f'
};

struct service_t {
    bk_error_t (*publish)  (const char* module_id, const char* meta,
                            const session_admin_t* session_admin);
    bk_error_t (*withdraw) (const char* module_id, const char* name);
    void       (*debug)    (grade_t grade, const char* text);
};

#ifdef __cplusplus
}
#endif

#endif // _SERVICE_H //
