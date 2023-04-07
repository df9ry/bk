#ifndef AX25BASE_AX25INVALIDFRAME_HPP
#define AX25BASE_AX25INVALIDFRAME_HPP

#include "ax25payload.hpp"

namespace AX25Base {

class AX25InvalidFrame: public AX25Payload
{
public:
    /// <summary>
    /// AX.25 Invalid Frame.
    /// </summary>
    AX25InvalidFrame(const OctetArray& octets, bool cmd, bool rsp);

    /// <summary>
    /// Get Frame Type.
    /// </summary>
    virtual ax25frame_t FrameType() const { return ax25frame_t::_INV; }

    /// <summary>
    /// ToString method.
    /// </summary>
    /// <param name="sb">String builder.</param>
    virtual void ToString(std::ostringstream& sb) const
    {
        sb << "IFRM " << m_payload->max_size() << " octets";
    }
};

} // namespace AX25Base

#endif // AX25BASE_AX25INVALIDFRAME_HPP
