#ifndef AX25BASE_AX25FRAME_HPP
#define AX25BASE_AX25FRAME_HPP

#include "ax25header.hpp"
#include "ax25payload.hpp"

#include <memory>

namespace AX25Base {

/// <summary>
/// A complete AX.25 Frame, fully decoded.
/// </summary>
class AX25Frame
{
public:
    typedef std::shared_ptr<AX25Frame> Ptr;

    /// <summary>
    /// Constructor
    /// </summary>
    /// <param name="frame">Frame octets.</param>
    /// <param name="modulo">AX.25 Modulo</param>
    AX25Frame(const OctetArray& frame, ax25modulo_t modulo = AX25Base::ax25modulo_t::MOD8);

    /// <summary>
    /// Constructor.
    /// </summary>
    /// <param name="header">AX.25 header.</param>
    /// <param name="payload">Paylaod data.</param>
    /// <param name="modulo">AX.25 Modulo</param>
    AX25Frame(AX25Header::Ptr header, const OctetArray& payload, ax25modulo_t modulo);

    /// <summary>
    /// Constructor.
    /// </summary>
    /// <param name="header">AX.25 header.</param>
    /// <param name="payload">AX.25 payload</param>
    AX25Frame(AX25Header::Ptr header, AX25Payload::Ptr payload);

    /// <summary>
    /// Command?
    /// </summary>
    bool IsCommand() const { return Header->IsCommand(); }

    /// <summary>
    /// Response?
    /// </summary>
    bool IsResponse() const { return Header->IsResponse(); }

    /// <summary>
    /// Old version 1.x frame?
    /// </summary>
    bool IsV1Frame() const { return Header->IsV1Frame(); }

    /// <summary>
    /// Recent version 2.x frame?
    /// </summary>
    bool IsV2Frame() const { return Header->IsV2Frame(); }

    /// <summary>
    /// Frame octets.
    /// </summary>
    const OctetArray& GetOctets() const;

    /// <summary>
    /// The ToString() method.
    /// </summary>
    /// <returns>Human readable representation of this object.</returns>
    virtual std::string ToString()
        { return Header->ToString() + " " + Payload->ToString(); }

    /// <summary>
    /// Frame heaader.
    /// </summary>
    AX25Header::Ptr GetHeader() const { return Header; }

    /// <summary>
    /// Frame payload.
    /// </summary>
    AX25Payload::Ptr GetPayload() const { return Payload; }

private:
    AX25Header::Ptr    Header;
    AX25Payload::Ptr   Payload;
    mutable OctetArray CashedOctets{nullptr};
};

} // namespace AX25Base

#endif // AX25BASE_AX25FRAME_HPP
