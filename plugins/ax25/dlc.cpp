#include "dlc.hpp"
#include "session.hpp"

namespace ax25 {

    DLC::DLC(const Session &_mySession): mySession{_mySession}
    {
    }

    void DLC::receive(Element::Ptr_t e)
    {

    }

    void DLC::transmit(Element::Ptr_t e)
    {

    }

} // end namespace ax25 //
