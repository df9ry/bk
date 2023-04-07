#include "ax25_i.hpp"
#include "ax25exceptions.hpp"

#include <cstring>

namespace AX25Base {

AX25_I::AX25_I(const OctetArray& i, ax25modulo_t modulo, int n_r, int n_s, bool p,
               bool cmd, bool rsp)
    : AX25Payload(OctetArray(new octet_vector_t(ModuloSize(modulo) + i->size())),
                modulo, cmd, rsp)
    {
        m_payload->at(0) = 0x00;
        set_I(i);
        set_N_R(n_r);
        set_N_S(n_s);
        set_P(p);
    }

    AX25_I::AX25_I(const OctetArray& octets, ax25modulo_t modulo, bool cmd, bool rsp)
        : AX25Payload(octets, modulo, cmd, rsp)
    {
    }

    ax25frame_t AX25_I::FrameType() const
    {
        return ax25frame_t::I;
    }

    bool AX25_I::get_P() const
    {
        switch (m_modulo)
        {
        case ax25modulo_t::MOD8: return ((m_payload->at(0) & 0x10) != 0x00);
        case ax25modulo_t::MOD128: return ((m_payload->at(0) & 0x01) != 0x10);
        default:
            throw ArgumentOutOfRangeException("Attempt to get P bit with unknown modulo");
        } // end switch //
    }

    void AX25_I::set_P(bool value)
    {
        switch (m_modulo)
        {
        case ax25modulo_t::MOD8:
            if (value)
                m_payload->at(0) = (octet_t)(m_payload->at(0) | 0x10);
            else
                m_payload->at(0) = (octet_t)(m_payload->at(0) & 0xEF);
            break;
        case ax25modulo_t::MOD128:
            if (value)
                m_payload->at(1) = (octet_t)(m_payload->at(1) | 0x01);
            else
                m_payload->at(1) = (octet_t)(m_payload->at(1) & 0xFE);
            break;
        default:
            throw ArgumentOutOfRangeException("Attempt to set P bit with unknown modulo");
        } // end switch //
    }

    int AX25_I::get_N_R() const
    {
        switch (m_modulo)
        {
        case ax25modulo_t::MOD8: return (m_payload->at(0) & 0xE0) >> 5;
        case ax25modulo_t::MOD128: return (m_payload->at(1) & 0xFE) >> 1;
        default:
            throw new ArgumentOutOfRangeException("Attempt to get N_R with unknown modulo");
        } // end switch //
    }

    void AX25_I::set_N_R(int value)
    {
        switch (m_modulo)
        {
        case ax25modulo_t::MOD8:
            m_payload->at(0) = (octet_t)(m_payload->at(0) & 0x1F);
            m_payload->at(0) |= (octet_t)((value & 0x07) << 5);
            break;
        case ax25modulo_t::MOD128:
            m_payload->at(1) = (octet_t)(m_payload->at(1) & 0x01);
            m_payload->at(1) |= (octet_t)((value & 0x7F) << 1);
            break;
        default:
            throw new ArgumentOutOfRangeException("Attempt to set N_R with unknown modulo");
        } // end switch //
    }

    int AX25_I::get_N_S() const
    {
        switch (m_modulo)
        {
        case ax25modulo_t::MOD8: return (m_payload->at(0) & 0x0E) >> 1;
        case ax25modulo_t::MOD128: return (m_payload->at(0) & 0xFE) >> 1;
        default:
            throw new ArgumentOutOfRangeException("Attempt to get N_S with unknown modulo");
        } // end switch //
    }

    void AX25_I::set_N_S(int value)
    {
        switch (m_modulo)
        {
        case ax25modulo_t::MOD8:
            m_payload->at(0) = (octet_t)(m_payload->at(0) & 0xF1);
            m_payload->at(0) |= (octet_t)((value & 0x07) << 1);
            break;
        case ax25modulo_t::MOD128:
            m_payload->at(0) = (octet_t)(m_payload->at(0) & 0x01);
            m_payload->at(0) |= (octet_t)((value & 0x7F) << 1);
            break;
        default:
            throw new ArgumentOutOfRangeException("Attempt to set N_S with unknown modulo");
        } // end switch //
    }

    OctetArray AX25_I::get_I() const
    {
        octet_t* p;
        size_t l;
        switch (m_modulo)
        {
        case ax25modulo_t::MOD8:
            p = &m_payload->at(1);
            l = m_payload->size() - 1;
            break;
        case ax25modulo_t::MOD128:
            p = &m_payload->at(2);
            l = m_payload->size() - 2;
            break;
        default:
            p = &m_payload->at(0);
            l = m_payload->size();
            break;
        } // end switch //
        OctetArray value{new octet_vector_t(p, p+l)};
        return value;
    }

    void AX25_I::set_I(const OctetArray& value) {
        size_t s_hdr = ModuloSize(m_modulo);
        size_t s_dat = value->size();
        size_t s_tot = s_hdr + s_dat;
        if (s_tot != m_payload->size()) {
            OctetArray octets{new octet_vector_t(s_tot)};
            ::memcpy(octets->data(), m_payload->data(), s_hdr);
            m_payload = octets;
        }
        ::memcpy(value->data(), &m_payload->at(s_hdr), s_dat);
    }

    int AX25_I::InfoFieldLength() const
    {
        return m_payload->size() - ModuloSize(m_modulo);
    }

    void AX25_I::ToString(std::ostringstream& sb)
    {
        sb << "I(R=" << get_N_R() << ",S=" << get_N_S() << (get_P()?",P":"") << ") "
           << get_I()->size() << " octets";
    }
} // namespace AX25Base
