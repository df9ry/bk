#ifndef AX25BASE_AX25_I_HPP
#define AX25BASE_AX25_I_HPP

#include "ax25payload.hpp"

namespace AX25Base {

/// <summary>
/// AX.25 I Frame.
/// </summary>
class AX25_I: public AX25Payload
{
public:
    /// <summary>
    /// Constructor.
    /// </summary>
    /// <param name="i">Information field.</param>
    /// <param name="modulo">Modulo.</param>
    /// <param name="n_r">N(r).</param>
    /// <param name="n_s">N(s).</param>
    /// <param name="p">Poll?</param>
    /// <param name="cmd">Command?</param>
    /// <param name="rsp">Response?</param>
    AX25_I(const OctetArray& i, ax25modulo_t modulo, int n_r, int n_s, bool p,
           bool cmd = true, bool rsp = false);

    /// <summary>
    /// Constructor.
    /// </summary>
    /// <param name="octets">Complete frame.</param>
    /// <param name="modulo">Modulo.</param>
    /// <param name="cmd">Command?</param>
    /// <param name="rsp">Response?</param>
    AX25_I(const OctetArray& octets, ax25modulo_t modulo, bool cmd, bool rsp);

    /// <summary>
    /// Get Frame Type.
    /// </summary>
    virtual ax25frame_t FrameType() const;

    /// <summary>
    /// Information field.
    /// </summary>
    OctetArray get_I() const;
    void set_I(const OctetArray& value);

    /// <summary>
    /// Length of the info field.
    /// </summary>
    int InfoFieldLength() const;

    /// <summary>
    /// Poll bit.
    /// </summary>
    bool get_P() const;
    void set_P(bool value);

    /// <summary>
    /// N(r).
    /// </summary>
    int  get_N_R() const;
    void set_N_R(int value);

    /// <summary>
    /// N(s).
    /// </summary>
    int  get_N_S() const;
    void set_N_S(int value);

protected:
    /// <summary>
    /// To String method.
    /// </summary>
    /// <param name="sb">StringBuilder.</param>
    virtual void ToString(std::ostringstream& sb);
};

} // namespace AX25Base

#endif // AX25BASE_AX25_I_HPP
