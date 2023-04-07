#ifndef _AX25_BASE_AX25TYPES_HPP
#define _AX25_BASE_AX25TYPES_HPP

#include <cstddef>
#include <cstdint>
#include <vector>
#include <memory>
#include <string>

namespace AX25Base {

typedef uint8_t octet_t;
typedef std::vector<octet_t> octet_vector_t;
typedef std::shared_ptr<octet_vector_t> OctetArray;

/// <summary>
/// AX.25 Frame Type.
/// </summary>
typedef enum
{
    /** <summary> RR            </summary> */ RR    = 0x00,
    /** <summary> I             </summary> */ I     = 0x01,
    /** <summary> UI            </summary> */ UI    = 0x03,
    /** <summary> RNR           </summary> */ RNR   = 0x04,
    /** <summary> REJ           </summary> */ REJ   = 0x08,
    /** <summary> SREJ          </summary> */ SREJ  = 0x0c,
    /** <summary> DM            </summary> */ DM    = 0x0f,
    /** <summary> SABM          </summary> */ SABM  = 0x2f,
    /** <summary> DISC          </summary> */ DISC  = 0x43,
    /** <summary> UA            </summary> */ UA    = 0x63,
    /** <summary> SABME         </summary> */ SABME = 0x6f,
    /** <summary> FRMR          </summary> */ FRMR  = 0x87,
    /** <summary> XID           </summary> */ XID   = 0xaf,
    /** <summary> TEST          </summary> */ TEST  = 0xe3,
    /** <summary> Invalid frame </summary> */ _INV  = 0xff,
} ax25frame_t;
const char* ax25frame_s(ax25frame_t t);
inline int  ax25frame_mask(ax25frame_t t) { return static_cast<int>(t); };

/// <summary>
/// AX.25 Modulo indicator.
/// </summary>
typedef enum {
    /** <summary>Modulo not specified     </summary>*/ UNSPECIFIED = 0,
    /** <summary>Modulo is 3 bits [0..7]  </summary>*/ MOD8 = 8,
    /** <summary>Modulo is 7 bits [0..127]</summary>*/ MOD128 = 128
} ax25modulo_t;
const char* ax25modulo_s(ax25modulo_t t);

/// <summary>
/// AX.25 Version indicator.
/// </summary>
typedef enum {
    /** <summary>AX.25 Version 2.0</summary> */ V2_0 = 20,
    /** <summary>AX.25 Version 2.2</summary> */ V2_2 = 22
} ax25version_t;
const char* ax25version_s(ax25version_t t);

} // end namespace AX25Base //

#endif // _AX25_BASE_AX25TYPES_HPP
