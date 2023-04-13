#include "l2callsign.hpp"
#include "ax25exceptions.hpp"

#include <string>
#include <sstream>
#include <algorithm>

using namespace std;

static inline string to_upper(string input) {
    transform(input.begin(), input.end(), input.begin(), ::toupper);
    return input;
}

namespace AX25Base {

    const L2Callsign L2Callsign::CQ{L2Callsign("CQ")};

    L2Callsign::L2Callsign(OctetArray octets, size_t iStart, bool& last)
    {
    int l = octets->size();
        if ((iStart < 0) || (iStart + 7 > l))
            throw new ArgumentOutOfRangeException("octets");
        ostringstream sb;
        for (int i = 0; i < 6; ++i) {
            octet_t b = octets->at(iStart + i);
            if ((b & 0x01) != 0x00)
                throw new InvalidAX25FrameException("EOA bit inside of callsign field");
            int _ch = ((int)b) >> 1;
            if (_ch == (int)' ')
                break;
            sb << ((char)_ch);
        } // end for //
        this->callsign = sb.str();
        octet_t c = octets->at(iStart + 6);
        last = ((c & 0x01) != 0x00);
        this->chBit = ((c & 0x80) != 0x00);
        this->ssid = (int)((c & 0x1e) >> 1);
    }

    L2Callsign::L2Callsign(const string& callsign, int ssid, bool chBit)
    {
        if (callsign.empty())
            throw new InvalidPropertyException("Empty callsign");
        if (callsign.length() > 6)
            throw new InvalidPropertyException("Callsign too long: \"" + callsign + "\"");
        this->callsign = to_upper(callsign);
        if ((ssid < 0) || (ssid > 15))
            throw new InvalidPropertyException("SSID range error [0..15]: "
                                               + std::to_string(ssid));
        this->ssid = ssid;
        this->chBit = chBit;
    }

    L2Callsign::L2Callsign(const string& callsign, bool chBit)
    {
        string _callsign;
        int _ssid;

        if (callsign.empty())
            throw new InvalidPropertyException("Empty callsign");

        auto p = callsign.find('-');
        if (p != string::npos)
        {
            _callsign = callsign.substr(0, p);
            _ssid = stoi(callsign.substr(p+1));
        } else {
            _callsign = callsign;
            _ssid = 0;
        }
        if (_callsign.length() > 6)
            throw new InvalidPropertyException("Callsign too long: \"" + _callsign + "\"");
        this->callsign = to_upper(_callsign);
        if ((_ssid < 0) || (_ssid > 15))
            throw new InvalidPropertyException("SSID range error [0..15]: "
                                               + std::to_string(_ssid));
        this->ssid = _ssid;
        this->chBit = chBit;
    }

    /// <summary>
    /// Copy  Constructor with CH bit override.
    /// </summary>
    /// <param name="callsign">Source callsign.</param>
    /// <param name="chBit">The CH bit.</param>
    L2Callsign::L2Callsign(const L2Callsign& other, bool chBit)
    {
        this->callsign = other.callsign;
        this->ssid = other.ssid;
        this->chBit = chBit;
    }

    OctetArray L2Callsign::octets() const
    {
        int l = callsign.length();
        OctetArray octets{new octet_vector_t(7)};
        const char* c = &callsign[0];
        int i = 0;
        for (; i < l; ++i)
            octets->at(i) = (octet_t)(((int)c[i]) << 1);
        for (; i < 6; ++i)
            octets->at(i) = 0x40;
        octets->at(6) = (octet_t)((ssid << 1) | ((chBit) ? 0xE0 : 0x60));
        return octets;
    }

    string L2Callsign::to_string() const
    {
        if (ssid)
            return callsign + "-" + std::to_string(ssid);
        else
            return callsign;
    }

} // end namespace AX25Base //

