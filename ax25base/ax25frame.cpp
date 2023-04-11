#include "ax25frame.hpp"

namespace AX25Base {

AX25Frame::AX25Frame(const OctetArray& frame, ax25modulo_t modulo):
    Header{AX25Header(frame)},
    Payload{AX25Payload::Create(frame, Header.Length(), Header.IsCommand(),
                                Header.IsResponse(), modulo)}
{
}

AX25Frame::AX25Frame(const AX25Header& header, const OctetArray& payload,
                     ax25modulo_t modulo):
    Header{header},
    Payload(AX25Payload::Create(payload, Header.IsCommand(), Header.IsResponse(), modulo))
{
}

AX25Frame::AX25Frame(const AX25Header& header, AX25Payload_ptr payload):
    Header{AX25Header(header, payload->Command(), payload->Response())},
    Payload{payload}
{
}

const OctetArray& AX25Frame::Octets() const
{
    if (CashedOctets->empty())
    {
        OctetArray header = Header.GetOctets();
        CashedOctets->insert(CashedOctets->end(), header->begin(), header->end());
        OctetArray payload = Payload->Octets();
        CashedOctets->insert(CashedOctets->end(), payload->begin(), payload->end());
    }
    return CashedOctets;
}

} // namespace AX25Base
