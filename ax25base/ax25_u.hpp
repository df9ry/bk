#ifndef AX25BASE_AX25_U_HPP
#define AX25BASE_AX25_U_HPP

#include "ax25payload.hpp"

namespace AX25Base {

/// <summary>
/// AX.25 Supervisory Frame.
/// </summary>
class AX25_U: public AX25Payload
{
public:
    static AX25Payload_ptr Create(const OctetArray& frame, bool cmd, bool rsp);

    /// <summary>
    /// Poll / Final bit.
    /// </summary>
    bool get_PF() const;
    void set_PF(bool value);

    /// <summary>
    /// Information field.
    /// </summary>
    OctetArray get_I() const;
    void set_I(const OctetArray& value);

    /// <summary>
    /// Length of the info field.
    /// </summary>
    int InfoFieldLength() const;

protected:
    /// <summary>
    /// Constructor.
    /// </summary>
    /// <param name="payload">Data bytes.</param>
    /// <param name="cmd">Command?</param>
    /// <param name="rsp">Respose?</param>
    AX25_U(const OctetArray& payload, bool cmd, bool rsp);
};

template<ax25frame_t N>
class AX25_UX: public AX25_U {
public:
    /// <summary>
    /// Constructor.
    /// </summary>
    /// <param name="p">Poll bit.</param>
    /// <param name="cmd">Command?</param>
    /// <param name="rsp">Response?</param>
    AX25_UX(bool p, bool cmd = true, bool rsp = false)
        : AX25_U(OctetArray(new octet_vector_t(1)), cmd, rsp)
    {
        m_payload[0] = ax25frame_mask(N);
        set_PF(p);
    }

    /// <summary>
    /// Constructor.
    /// </summary>
    /// <param name="octets">Frame</param>
    /// <param name="cmd">Command?</param>
    /// <param name="rsp">Response?</param>
    AX25_UX(const OctetArray& octets, bool cmd, bool rsp)
        : AX25_U(octets, cmd, rsp)
    {
    }

    /// <summary>
    /// Get Frame Type.
    /// </summary>
    virtual ax25frame_t FrameType() const { return N; }

    /// <summary>
    /// ToString method.
    /// </summary>
    /// <param name="sb">String builder.</param>
    virtual void ToString(std::ostringstream& sb)
    {
        sb << ax25frame_s(N);
        if (get_PF())
            sb << (Command() ? "(P)" : "(F)");
    }
};

typedef AX25_UX<ax25frame_t::SABME> AX25_SABME;
typedef AX25_UX<ax25frame_t::SABM>  AX25_SABM;
typedef AX25_UX<ax25frame_t::DISC>  AX25_DISC;
typedef AX25_UX<ax25frame_t::DM>    AX25_DM;
typedef AX25_UX<ax25frame_t::UA>    AX25_UA;
typedef AX25_UX<ax25frame_t::FRMR>  AX25_FRMR;
typedef AX25_UX<ax25frame_t::UI>    AX25_UI;
typedef AX25_UX<ax25frame_t::XID>   AX25_XID;
typedef AX25_UX<ax25frame_t::TEST>  AX25_TEST;

} // namespace AX25Base

#endif // AX25BASE_AX25_U_HPP
