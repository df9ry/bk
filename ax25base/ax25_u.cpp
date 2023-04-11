#include "ax25_u.hpp"
#include "ax25invalidframe.hpp"
#include "ax25exceptions.hpp"

#include <cstring>

namespace AX25Base {

    AX25Payload::Ptr AX25_U::Create(const OctetArray& frame, bool cmd, bool rsp)
    {
        switch (frame->at(0) & 0xef)
        {
        case SABME: return AX25Payload::Ptr(new AX25_SABME(frame, cmd, rsp));
        case SABM : return AX25Payload::Ptr(new AX25_SABM(frame, cmd, rsp));
        case DISC : return AX25Payload::Ptr(new AX25_DISC(frame, cmd, rsp));
        case DM   : return AX25Payload::Ptr(new AX25_DM(frame, cmd, rsp));
        case UA   : return AX25Payload::Ptr(new AX25_UA(frame, cmd, rsp));
        case FRMR : return AX25Payload::Ptr(new AX25_FRMR(frame, cmd, rsp));
        case UI   : return AX25Payload::Ptr(new AX25_UI(frame, cmd, rsp));
        case XID  : return AX25Payload::Ptr(new AX25_XID(frame, cmd, rsp));
        case TEST : return AX25Payload::Ptr(new AX25_TEST(frame, cmd, rsp));
        default: return AX25Payload::Ptr(new AX25InvalidFrame(frame, cmd, rsp));
        } // end switch //
    }

    AX25_U::AX25_U(const OctetArray& payload, bool cmd, bool rsp)
        : AX25Payload(payload, ax25modulo_t::UNSPECIFIED, cmd, rsp)
    {
    }

    bool AX25_U::get_PF() const
    {
        switch (m_modulo)
        {
        case ax25modulo_t::MOD8: return ((m_payload->at(0) & 0x10) != 0x00);
        case ax25modulo_t::MOD128: return ((m_payload->at(1) & 0x01) != 0x10);
        default:
            throw new InvalidPropertyException("Attempt to get PF with unknown modulo");
        } // end switch //
    }

    void AX25_U::set_PF(bool value)
    {
        if (value)
            m_payload->at(0) = (octet_t)(m_payload->at(0) | 0x10);
        else
            m_payload->at(0) = (octet_t)(m_payload->at(0) & 0xEF);
    }

    OctetArray AX25_U::get_I() const
    {
        octet_t* p = &m_payload->at(1);
        size_t   l = m_payload->size() - 1;
        OctetArray value{new octet_vector_t(p, p+l)};
        return value;
    }

    void AX25_U::set_I(const OctetArray& value) {
        size_t s_hdr = 1;
        size_t s_dat = value->size();
        size_t s_tot = s_hdr + s_dat;
        if (s_tot != m_payload->size()) {
            OctetArray octets{new octet_vector_t(s_tot)};
            ::memcpy(octets->data(), m_payload->data(), s_hdr);
            m_payload = octets;
        }
        ::memcpy(value->data(), &m_payload->at(s_hdr), s_dat);
    }

    int AX25_U::InfoFieldLength() const
    {
        return m_payload->size() - 1;
    }

} // namespace AX25Base
