#include "crcb.hpp"

/**
 * CRC_B encoding This annex is provided for explanatory purposes and indicates
 * the bit patterns that will exist in the physical layer. It is included for
 * the purpose of checking an ISO/IEC 14443-3 Type B implementation of CRC_B
 * encoding. Refer to ISO/IEC 3309 and CCITT X.25 2.2.7 and V.42 8.1.1.6.1 for
 * further details. Initial Value = 'FFFF'
 */

namespace CrcB {

    static const uint16_t crcBDefault = 0xffff;

    static uint16_t UpdateCrc(uint8_t b, uint16_t crc)
    {
        uint8_t ch = static_cast<uint8_t>(b^static_cast<uint8_t>(crc & 0x00ff));
        ch = static_cast<uint8_t>(ch ^ (ch << 4));
        return static_cast<uint16_t>((crc >> 8)^(ch << 8)^(ch << 3)^(ch >> 4));
    }

    uint16_t crc(const uint8_t *pb, size_t cb)
    {
        uint16_t crc{crcBDefault};
        for (; cb--; pb++)
            crc = UpdateCrc(*pb, crc);
        return static_cast<uint16_t>(~crc);
    }

} // end namespace CrcB //
