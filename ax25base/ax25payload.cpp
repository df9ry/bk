#include "ax25payload.hpp"
#include "ax25exceptions.hpp"
#include "ax25invalidframe.hpp"
#include "ax25_i.hpp"
#include "ax25_s.hpp"
#include "ax25_u.hpp"

namespace AX25Base {

AX25Payload::AX25Payload(const OctetArray& payload, ax25modulo_t modulo, bool cmd, bool rsp)
{
    if (!payload)
        throw NullPointerException("payload");
    m_payload  = payload;
    m_modulo   = modulo;
    m_command  = cmd;
    m_response = rsp;
}

AX25Payload::Ptr AX25Payload::Create(const OctetArray& payload, bool cmd, bool rsp,
                          ax25version_t version)
{
    if (!payload)
        throw NullPointerException("payload");
    ax25modulo_t modulo = (version == ax25version_t::V2_2) ?
                              ax25modulo_t::MOD128 : ax25modulo_t::MOD8;
    return AX25Payload::Create(payload, cmd, rsp, version);
}

AX25Payload::Ptr AX25Payload::Create(const OctetArray& frame, int iFrame, bool cmd, bool rsp,
                          ax25version_t version)
{
    if (!frame)
        throw NullPointerException("frame");
    ax25modulo_t modulo = (version == ax25version_t::V2_2) ?
                              ax25modulo_t::MOD128 : ax25modulo_t::MOD8;
    return AX25Payload::Create(frame, iFrame, cmd, rsp, version);
}

AX25Payload::Ptr Create(const OctetArray& payload, bool cmd, bool rsp,
                          ax25modulo_t modulo)
{
    if (!payload)
        throw NullPointerException("payload");
    if (payload->empty())
        return AX25Payload::Ptr(new AX25InvalidFrame(payload, cmd, rsp));
    if ((payload->at(0) & 0x01) == 0x00) // I-Frame:
        return AX25Payload::Ptr(new AX25_I(payload, modulo, cmd, rsp));
    if ((payload->at(0) & 0x02) == 0x00) // S-Frame:
        return AX25_S::Create(payload, modulo, cmd, rsp);
    else
        return AX25_U::Create(payload, cmd, rsp);
}

AX25Payload::Ptr AX25Payload::Create(const OctetArray& frame, int iFrame, bool cmd, bool rsp,
                           ax25modulo_t modulo)
{
    if (!frame)
        throw NullPointerException("frame");
    int lPayload = frame->size() - iFrame;
    if (lPayload < 1)
        throw InvalidAX25FrameException("Frame too short");
    auto p = &frame->at(iFrame);
    OctetArray payload = OctetArray(new octet_vector_t(p, p + lPayload));
    return AX25Payload::Create(payload, cmd, rsp, modulo);
}

int AX25Payload::ModuloSize(ax25modulo_t modulo)
{
    switch (modulo)
    {
    case ax25modulo_t::UNSPECIFIED: return 0;
    case ax25modulo_t::MOD8: return 1;
    case ax25modulo_t::MOD128: return 2;
    default: throw new ArgumentOutOfRangeException("modulo");
    } // end switch //
}

} // namespace AX25Base
