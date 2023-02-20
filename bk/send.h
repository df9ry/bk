#ifndef _SEND_H
#define _SEND_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#include "error.h"

struct send_t {
    bk_error_t (*send) (const char* head, const uint8_t* p_body, size_t c_body);
};

#ifdef __cplusplus
}
#endif

#endif // _SEND_H //
