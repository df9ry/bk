#ifndef ERROR_H
#define ERROR_H

typedef enum {
    BK_ERC_OK = 0,
    BK_ERC_PUBLISH,
    BK_ERC_NO_META,
    BK_ERC_INV_META,
    BK_ERC_NO_ID,
    BK_ERC_NO_SERVICE_IFC,
    BK_ERC_INV_SERVICE_IFC,
} bk_error_t;

#endif // ERROR_H
