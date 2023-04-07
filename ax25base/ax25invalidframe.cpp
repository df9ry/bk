#include "ax25invalidframe.hpp"

namespace AX25Base {

AX25InvalidFrame::AX25InvalidFrame(const OctetArray& octets, bool cmd, bool rsp)
    : AX25Payload(octets, UNSPECIFIED, cmd, rsp)
{
}

} // namespace AX25Base
