#ifndef _SO_H
#define _SO_H

#ifdef __cplusplus
extern "C" {
#endif

struct service_t {
    void* (*publish)  (const char* meta);
    bool (*withdraw) (const char* name);
};

#ifdef __cplusplus
}
#endif

#endif // _SO_H //