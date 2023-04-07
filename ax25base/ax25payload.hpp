#ifndef AX25BASE_AX25PAYLOAD_HPP
#define AX25BASE_AX25PAYLOAD_HPP

#include "ax25types.hpp"

#include <string>
#include <sstream>
#include <memory>

namespace AX25Base {

class AX25Payload;
typedef std::shared_ptr<AX25Payload> AX25Payload_ptr;

class AX25Payload
{
public:

    /// <summary>
    /// Create AX.25 element from frame data.
    /// </summary>
    /// <param name="payload">Data bytes.</param>
    /// <param name="cmd">Command?</param>
    /// <param name="rsp">Response?</param>
    /// <param name="version">AX.25 version to use.</param>
    /// <returns>AX.25 element.</returns>
    static AX25Payload_ptr Create(const OctetArray& payload, bool cmd, bool rsp,
                              ax25version_t version);

    /// <summary>
    /// Create AX.25 element from frame data.
    /// </summary>
    /// <param name="frame">Data bytes.</param>
    /// <param name="iFrame">Index where payload starts</param>
    /// <param name="cmd">Command?</param>
    /// <param name="rsp">Response?</param>
    /// <param name="version">AX.25 version to use.</param>
    /// <returns>AX.25 element.</returns>
    static AX25Payload_ptr Create(const OctetArray& frame, int iFrame, bool cmd, bool rsp,
                              ax25version_t version);

    /// <summary>
    /// Create AX.25 element from frame data.
    /// </summary>
    /// <param name="payload">Data bytes.</param>
    /// <param name="cmd">Command?</param>
    /// <param name="rsp">Response?</param>
    /// <param name="modulo">AX.25 Modulo</param>
    /// <returns>AX.25 element.</returns>
    static AX25Payload_ptr Create(const OctetArray& payload, bool cmd, bool rsp,
                               ax25modulo_t modulo);

    /// <summary>
    /// Create AX.25 element from frame data.
    /// </summary>
    /// <param name="frame">Data bytes.</param>
    /// <param name="iFrame">Index where payload starts</param>
    /// <param name="cmd">Command?</param>
    /// <param name="rsp">Response?</param>
    /// <param name="modulo">AX.25 Modulo</param>
    /// <returns>AX.25 element.</returns>
    static AX25Payload_ptr Create(const OctetArray& frame, int iFrame, bool cmd, bool rsp,
                               ax25modulo_t modulo);

    /// <summary>
    /// Size of the header for a AX.25 modulo.
    /// </summary>
    /// <param name="modulo">AX.25 Modulo.</param>
    /// <returns>Size of the modulo.</returns>
    static int ModuloSize(ax25modulo_t modulo);

    /// <summary>
    /// Frame Type.
    /// </summary>
    virtual ax25frame_t FrameType() const { return ax25frame_t::_INV; }

    /// <summary>
    /// Frame Type Name.
    /// </summary>
    std::string FrameTypeName() const { return ax25frame_s(FrameType()); }

    /// <summary>
    /// Command.
    /// </summary>
    bool Command() const { return (m_command && !m_response);}

    /// <summary>
    /// Response.
    /// </summary>
    bool Response() const { return (m_response && !m_command); }

    /// <summary>
    /// AX.25 V 1.x frame.
    /// </summary>
    bool PreviousVersion() const { return (m_command == m_response); }

    /// <summary>
    /// Get frame as byte array.
    /// </summary>
    const OctetArray& Octets() const { return m_payload; }

    /// <summary>
    /// To String method.
    /// </summary>
    /// <returns>String representation.</returns>
    std::string ToString() const
    {
        std::ostringstream sb;
        ToString(sb);
        return sb.str();
    }

protected:
    /// <summary>
    /// To String method.
    /// </summary>
    /// <param name="sb">StringBuilder.</param>
    virtual void ToString(std::ostringstream& sb) const { sb << "_INV"; }

    /// <summary>
    /// Constructor.
    /// </summary>
    /// <param name="payload">Data bytes.</param>
    /// <param name="modulo">AX.25 Modulo.</param>
    /// <param name="cmd">Command?</param>
    /// <param name="rsp">Response?</param>
    AX25Payload(const OctetArray& payload, ax25modulo_t modulo, bool cmd, bool rsp);

    /** <summary>Data bytes.  </summary> */ OctetArray     m_payload;
    /** <summary>AX.25 Modulo.</summary> */ ax25modulo_t   m_modulo;
    /** <summary>Command?     </summary> */ bool           m_command;
    /** <summary>Response?    </summary> */ bool           m_response;
};

} // namespace AX25Base

#endif // AX25BASE_AX25PAYLOAD_HPP
