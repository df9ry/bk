#ifndef _AX25BASE_L2CALLSIGN_HPP
#define _AX25BASE_L2CALLSIGN_HPP

#include "ax25types.hpp"

#include <string>

namespace AX25Base {

    /// <summary>
    /// Hamradio callsign as used in packet radio.
    /// </summary>
    class L2Callsign
    {
    public:
        /// <summary>
        /// The empty callsign.
        /// </summary>
        static const L2Callsign CQ;

        /// <summary>
        /// Get a callsign from a buffer at a specified index.
        /// </summary>
        /// <param name="octets">The octet buffer.</param>
        /// <param name="iStart">The position in the buffer where to read.</param>
        /// <param name="last"><c>true</c> if the EOA bit is set on this callsign.</param>
        L2Callsign(OctetArray octets, size_t iStart, bool& last);

        /// <summary>
        /// Construct a new callsign.
        /// </summary>
        /// <param name="callsign">The callsign without SSID.</param>
        /// <param name="ssid">The SSID.</param>
        /// <param name="chBit">The CH bit (Default: false).</param>
        L2Callsign(const std::string& callsign, int ssid, bool chBit);

        /// <summary>
        /// Copy constructor with CH bit override.
        /// </summary>
        /// <param name="callsign">Source callsign.</param>
        /// <param name="chBit">The CH bit.</param>
        L2Callsign(const std::string& callsign, bool chBit = false);

        /// <summary>
        /// Copy Constructor.
        /// </summary>
        /// <param name="callsign">Source callsign.</param>
        L2Callsign(const L2Callsign& other) = default;

        /// <summary>
        /// Copy  Constructor with CH bit override.
        /// </summary>
        /// <param name="callsign">Source callsign.</param>
        /// <param name="chBit">The CH bit.</param>
        L2Callsign(const L2Callsign& other, bool chBit);

        /// <summary>
        /// Get binary presentation of this callsign.
        /// </summary>
        OctetArray octets() const;

        /// <summary>
        /// Get string representation of this callsign.
        /// </summary>
        /// <returns>String representation of the callsign.</returns>
        std::string to_string() const;

        /// <summary>
        /// The C - bit (Poll / Final) or the H - bit (Has been repeated).
        /// </summary>
        bool get_chBit() const { return chBit; }

        /// <summary>
        /// The ssid.
        /// </summary>
        int get_ssid() const { return ssid; }

        /// <summary>
        /// Copy assignment operator.
        /// </summary>
        L2Callsign& operator=(const L2Callsign& other) = default;

    private:
        /// <summary>
        /// The callsign without(!) ssid.
        /// </summary>
        std::string callsign;

        /// <summary>
        /// The C - bit (Poll / Final) or the H - bit (Has been repeated).
        /// </summary>
        bool chBit;

        /// <summary>
        /// The ssid.
        /// </summary>
        int ssid;
    };

} // end namespace AX25Base //

#endif // _AX25BASE_L2CALLSIGN_HPP
