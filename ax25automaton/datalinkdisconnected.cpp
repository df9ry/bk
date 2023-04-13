#include "ifc.hpp"
#include "primitive.hpp"

#include <cassert>

namespace AX25Automaton {

void Automaton::dataLinkDisconnected(AbstractPrimitive& primitive, bool expedited)
{
    switch (primitive.kind) {
    case AbstractPrimitive::Kind_t::AX25Frame:
        currentAX25inputFrame = static_cast<AX25FramePrimitive&>(primitive).ptr;
        inputIsExpedited = expedited;
        switch (currentAX25inputFrame->GetPayload()->FrameType()) {
        case AX25Base::ax25frame_t::UA:
            task = 4;
            break;
        case AX25Base::ax25frame_t::DM:
            task = 5;
            break;
        case AX25Base::ax25frame_t::UI:
            task = 6;
            break;
        case AX25Base::ax25frame_t::DISC:
            task = 8;
            break;
        case AX25Base::ax25frame_t::SABM:
            task = 14;
            break;
        case AX25Base::ax25frame_t::SABME:
            task = 15;
            break;
        default:
            if (currentAX25inputFrame->GetPayload()->Command())
                task = 10;
            else
                task = 0;
        } // end switch //
        break;
    default:
        task = 0;
    } // end switch kind //
    while (task)
        exec();
}

} // end namespace AX25Automaton //
