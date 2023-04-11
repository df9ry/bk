#ifndef AX25BASE_AX25HEADER_HPP
#define AX25BASE_AX25HEADER_HPP

#include "ax25types.hpp"
#include "l2callsign.hpp"

#include <memory>
#include <strstream>
#include <string>

namespace AX25Base {

/// <summary>
/// Source, destination and digi infos for a L2 frame.
/// </summary>
class AX25Header
{
public:
    typedef std::shared_ptr<AX25Header> Ptr;

    /// <summary>
    /// Build the header from the frame data.
    /// </summary>
    /// <param name="frame"></param>
    AX25Header(OctetArray frame);

    /// <summary>
    /// Build the header from address information.
    /// </summary>
    /// <param name="source">The source address.</param>
    /// <param name="destination">The destination address.</param>
    /// <param name="digis">The intermediate digis.</param>
    AX25Header(const L2Callsign& source, const L2Callsign& destination,
               const std::vector<L2Callsign>& digis = {});

    /// <summary>
    /// Build the header from a template header.
    /// </summary>
    /// <param name="_template">Template header.</param>
    /// <param name="command">Command bit.</param>
    /// <param name="response">Response bit.</param>
    AX25Header(const AX25Header& _template, bool command, bool response);

    /// <summary>
    /// The source of the frame (where it comes from).
    /// </summary>
    L2Callsign Source() const;

    /// <summary>
    /// The destination of the frame (where it should arrive).
    /// </summary>
    L2Callsign Destination() const;

    /// <summary>
    /// The intermediate digis on the path.
    /// </summary>
    std::vector<L2Callsign> Digis() const;

    /// <summary>
    /// C-Bit of the destination callsign.
    /// </summary>
    bool IsCommand() const;

    /// <summary>
    /// C-Bit of the source callsign.
    /// </summary>
    bool IsResponse() const;

    /// <summary>
    /// Length of header data [octets].
    /// </summary>
    int Length() const;

    /// <summary>
    /// Test if this frame is a V1 frame.
    /// </summary>
    bool IsV1Frame() const { return (IsCommand() == IsResponse()); }

    /// <summary>
    /// Test if this frame is a V2 frame.
    /// </summary>
    bool IsV2Frame() const { return (IsCommand() != IsResponse()); }

    /// <summary>
    /// The next station to go.
    /// </summary>
    const L2Callsign& NextHop() const;

    /// <summary>
    /// Check if the frame has passed all digipeaters, if any.
    /// </summary>
    /// <returns>If all digipeaters are passed.</returns>
    bool MustDigi() const;

    /// <summary>
    /// Do hop.
    /// </summary>
    /// <returns></returns>
    void Digipeat();

    /// <summary>
    /// Get binary presentation of this header.
    /// </summary>
    OctetArray GetOctets() const;

    /// <summary>
    /// Fill in frame with header data.
    /// </summary>
    /// <param name="frame">The frame to set the data to.</param>
    void FillInOctetArray(OctetArray& frame) const;

    /// <summary>
    /// Get a string representation of this header.
    /// </summary>
    /// <returns>String representation of this header.</returns>
    virtual std::string ToString() const;

private:
    L2Callsign              destination;
    L2Callsign              source;
    std::vector<L2Callsign> digis;
    bool                    command;
    bool                    response;
    size_t                  length;
    bool                    last;
};

} // namespace AX25Base

#endif // AX25BASE_AX25HEADER_HPP
