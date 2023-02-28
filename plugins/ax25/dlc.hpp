#ifndef DLC_HPP
#define DLC_HPP

#include "elements.hpp"

class Session;

namespace ax25 {

    class DLC
    {
    public:
        DLC(const Session& mySession);

        void transmit(Element::Ptr_t e);
        void receive(Element::Ptr_t e);

    private:
        const Session& mySession;
    };

} // end namespace ax25 //

#endif // DLC_HPP
