#ifndef ELEMENTS_HPP
#define ELEMENTS_HPP

#include <memory>

namespace ax25 {

    class Element {
    public:
        enum class Type_t {
            // DLSAP group:
            DL_CONNECT_Request,
            DL_CONNECT_Indication,
            DL_CONNECT_Confirm,
            DL_DISCONNECT_Request,
            DL_DISCONNECT_Indication,
            DL_DISCONNECT_Confirm,
            DL_DATA_Request,
            DL_DATA_Indication,
            DL_UNIT_DATA_Request,
            DL_UNIT_DATA_Indication,
            DL_ERROR_Indication,
            DL_FLOW_OFF_Request,
            DL_FLOW_ON_Request,
            MDL_NEGOTIATE_Request,
            MDL_NEGOTIATE_Confirm,
            MDL_ERROR_Indication
        } type;

        typedef std::shared_ptr<Element> Ptr_t;
    };

}; // end namespace ax25 //

#endif // ELEMENTS_HPP
