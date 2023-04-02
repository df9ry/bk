#ifndef CRCB_HPP
#define CRCB_HPP

#include <cstdint>
#include <stddef.h>

namespace CrcB {

extern uint16_t crc(const uint8_t *pb, size_t cb);

} // end namespace CrcB //

#endif // CRCB_HPP
