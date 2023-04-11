#include "ax25types.hpp"

namespace AX25Base {

const char* ax25frame_s(ax25frame_t t)
{
#define CASE(N) case N: return #N
    switch (t) {
        CASE(_INV);
        CASE(SABME);
        CASE(SABM);
        CASE(DISC);
        CASE(DM);
        CASE(FRMR);
        CASE(REJ);
        CASE(RNR);
        CASE(RR);
        CASE(SREJ);
        CASE(UA);
        CASE(I);
        CASE(UI);
        CASE(TEST);
        CASE(XID);
    default:
        return "???";
    } // end switch //
#undef CASE
}

const char* ax25modulo_s(ax25modulo_t t)
{
#define CASE(X) case X: return #X
    switch (t) {
        CASE(UNSPECIFIED);
        CASE(MOD8);
        CASE(MOD128);
    default:
        return "???";
    } // end switch //
#undef CASE
}

const char* ax25version_s(ax25version_t t)
{
#define CASE(X) case X: return #X
    switch (t) {
        CASE(V2_0);
        CASE(V2_2);
    default:
        return "???";
    } // end switch //
#undef CASE
}

} // end namespace AX25Base //
