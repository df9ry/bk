#ifndef AGW_STRUCT_HPP
#define AGW_STRUCT_HPP

#include <stdint.h>

enum agw_frame_kind {
    UNPROTO        = 'U',
    TX_DATA        = 'T',
    SUPERVISORY    = 'S',
    RX_DATA        = 'I',
    RAW_FRAME      = 'K',
    RAW_SWITCH     = 'k',
    CONNECT        = 'C', // *** CONNECTED To Station ... or *** CONNECTED With ...
    CONNECT_PID    = 'c',
    CONNECT_V      = 'v', // Connect via ...
    DISCONNECT     = 'd',
    TX_QUEUE_S     = 'Y', // Outstanding frames for a station
    TX_QUEUE_P     = 'y', // Outstanding frames for a port
    PORT_INFO      = 'G',
    MHEARD_LIST    = 'H',
    PORT_CAPA      = 'g',
    VERSION        = 'R',
    REGISTER_CALL  = 'X',
    UNREGISTER_CALL= 'x',
    DATA_PROTO     = 'D',
    DATA_UNPROTO   = 'M',
    DATA_UNPROTO_V = 'V',
};

struct __attribute__((__packed__)) agw_header_t {
    uint32_t port;
    uint32_t kind;
    char     call_from[10];
    char     call_to[10];
    uint32_t data_length;
    uint32_t user;
};

#endif // AGW_STRUCT_HPP
