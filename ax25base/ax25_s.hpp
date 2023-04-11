#ifndef AX25BASE_AX25_S_HPP
#define AX25BASE_AX25_S_HPP

#include "ax25payload.hpp"

namespace AX25Base {

/// <summary>
/// AX.25 Supervisory Frame.
/// </summary>
class AX25_S: public AX25Payload
{
public:
    static AX25Payload::Ptr Create(const OctetArray& frame, ax25modulo_t modulo, bool cmd,
                                  bool rsp);

    /// <summary>
    /// Poll / Final bit.
    /// </summary>
    bool get_PF() const;
    void set_PF(bool value);

    /// <summary>
    /// N(r).
    /// </summary>
    int  get_N_R() const;
    void set_N_R(int value);

protected:
    /// <summary>
    /// Constructor.
    /// </summary>
    /// <param name="payload">Data bytes.</param>
    /// <param name="modulo">Modulo.</param>
    /// <param name="cmd">Command?</param>
    /// <param name="rsp">Respose?</param>
    AX25_S(const OctetArray& payload, ax25modulo_t modulo, bool cmd, bool rsp);
};

template<ax25frame_t N>
class AX25_SX: public AX25_S {
public:
    /// <summary>
    /// Constructor.
    /// </summary>
    /// <param name="modulo">AX.25 Modulo.</param>
    /// <param name="n_r">N(r).</param>
    /// <param name="pf">Poll / Final bit.</param>
    /// <param name="cmd">Command?</param>
    /// <param name="rsp">Response?</param>
    AX25_SX(ax25modulo_t modulo, int n_r, bool pf, bool cmd = true, bool rsp = false)
        : AX25_S(OctetArray(new octet_vector_t(ModuloSize(modulo))), modulo, cmd, rsp)
    {
        m_payload->at(0) = ax25frame_mask(N);
        set_N_R(n_r);
        set_PF(pf);
    }

    /// <summary>
    /// Constructor.
    /// </summary>
    /// <param name="octets">Frame data.</param>
    /// <param name="modulo">AX.25 Modulo.</param>
    /// <param name="cmd">Command?</param>
    /// <param name="rsp">Response?</param>
    AX25_SX(const OctetArray& octets, ax25modulo_t modulo, bool cmd, bool rsp)
        : AX25_S(octets, modulo, cmd, rsp)
    {
    }

    /// <summary>
    /// Get Frame Type.
    /// </summary>
    virtual ax25frame_t FrameType() const { return N; }

protected:
    /// <summary>
    /// ToString method.
    /// </summary>
    /// <param name="sb">String builder.</param>
    virtual void ToString(std::ostringstream& sb)
    {
        sb << ax25frame_s(N) << "(R=" << get_N_R();
        if (get_PF())
        {
            sb << "," << (Command() ? "P" : "F");
        }
        sb << ")";
    }
};

typedef AX25_SX<ax25frame_t::RR>   AX25_RR;
typedef AX25_SX<ax25frame_t::RNR>  AX25_RNR;
typedef AX25_SX<ax25frame_t::REJ>  AX25_REJ;
typedef AX25_SX<ax25frame_t::SREJ> AX25_SREJ;

} // namespace AX25Base

#endif // AX25BASE_AX25_S_HPP
