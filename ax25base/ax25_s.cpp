#include "ax25_s.hpp"
#include "ax25invalidframe.hpp"
#include "ax25exceptions.hpp"

#include <cstring>

namespace AX25Base {

    AX25Payload::Ptr AX25_S::Create(const OctetArray& frame, ax25modulo_t modulo, bool cmd,
                               bool rsp)
    {
        switch (((modulo != ax25modulo_t::MOD128)?(frame->at(0) & 0x0C):frame->at(0)))
        {
        case RR:   return AX25Payload::Ptr(new AX25_RR(frame, modulo, cmd, rsp));
        case RNR:  return AX25Payload::Ptr(new AX25_RNR(frame, modulo, cmd, rsp));
        case REJ:  return AX25Payload::Ptr(new AX25_REJ(frame, modulo, cmd, rsp));
        case SREJ: return AX25Payload::Ptr(new AX25_SREJ(frame, modulo, cmd, rsp));
        default: return AX25Payload::Ptr(new AX25InvalidFrame(frame, cmd, rsp));
        } // end switch //
    }

    AX25_S::AX25_S(const OctetArray& payload, ax25modulo_t modulo, bool cmd, bool rsp)
        : AX25Payload(payload, modulo, cmd, rsp)
    {
    }

    bool AX25_S::get_PF() const
    {
        switch (m_modulo)
        {
        case ax25modulo_t::MOD8: return ((m_payload->at(0) & 0x10) != 0x00);
        case ax25modulo_t::MOD128: return ((m_payload->at(1) & 0x01) != 0x10);
        default:
            throw new InvalidPropertyException("Attempt to get PF with unknown modulo");
        } // end switch //
    }

    void AX25_S::set_PF(bool value)
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
            throw new InvalidPropertyException("Attempt to sett PF with unknown modulo");
        } // end switch //
    }

    int AX25_S::get_N_R() const
    {
        switch (m_modulo)
        {
        case ax25modulo_t::MOD8: return (m_payload->at(0) & 0xE0) >> 5;
        case ax25modulo_t::MOD128: return (m_payload->at(1) & 0xFE) >> 1;
        default:
            throw new InvalidPropertyException("Attempt to get N_R with unknown modulo");
        } // end switch //
    }

    void AX25_S::set_N_R(int value)
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
            throw new InvalidPropertyException("Attempt to set N_R with unknown modulo");
        } // end switch //
    }

} // namespace AX25Base
