#include "bkerror.hpp"

#define CASE(X) case X: return #X

namespace BkBase {

    const char* bk_error_message(bk_error_t erc)
    {
        switch (erc) {
        CASE(BK_ERC_OK);
        CASE(BK_ERC_ENGAGED);
        CASE(BK_ERC_NOT_CONNECTED);
        CASE(BK_ERC_PUBLISH);
        CASE(BK_ERC_NO_META);
        CASE(BK_ERC_INV_META);
        CASE(BK_ERC_NO_ID);
        CASE(BK_ERC_NO_SERVICE_IFC);
        CASE(BK_ERC_INV_SERVICE_IFC);
        CASE(BK_ERC_NO_SESSION_IFC);
        CASE(BK_ERC_NO_SESSION_IFC_PTR);
        CASE(BK_ERC_NO_SESSION_ID_PTR);
        CASE(BK_ERC_NO_SUCH_SERVICE);
        CASE(BK_ERC_TO_MUCH_SESSIONS);
        CASE(BK_ERC_NO_SUCH_SESSION);
        CASE(BK_ERC_NOT_IMPLEMENTED);
        CASE(BK_ERC_INV_IP_VERSION);
        CASE(BK_ERC_INV_ADDR_INFO);
        CASE(BK_ERC_NO_ADDR_INFO);
        CASE(BK_ERC_SOCKET_ERROR);
        CASE(BK_ERC_BIND_ERROR);
        CASE(BK_ERC_LISTEN_ERROR);
        CASE(BK_ERC_TALK_ERROR);
        CASE(BK_ERC_ACCEPT_ERROR);
        CASE(BK_ERC_NULL_BODY);
        CASE(BK_ERC_INV_CRC_TYPE);
        CASE(BK_ERC_RUNTIME_EXCEPTION);
        default: return "BK_INVALID_ERROR_NUMBER";
        } // end switch //
    }

} // end namespace BkBase //
