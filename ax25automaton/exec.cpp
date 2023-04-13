#include "ifc.hpp"

#include <ax25base/ax25_u.hpp>

#include <cassert>

using namespace AX25Base;

namespace AX25Automaton {

void Automaton::exec()
{
    switch (task) {
    case 0:
        return;
    case 1: // Control field error
        // DL_ErrorIndication(L)
        task = 0;
        return;
    case 2: // Info not permitted in frame
        // DL_ErrorIndication(M)
        task = 0;
        return;
    case 3: // Incorrect U or S frame length
        // DL_ErrorIndication(N)
        task = 0;
        return;
    case 4: // Received UA
        // DL_ErrorIndication(C, D)
        task = 0;
        return;
    case 5: // Received DM
        task = 0;
        return;
    case 6: // Received UI
        UI_Check();
        if (currentAX25inputFrame->GetPayload()->get_PF()) {
            currentAX25outputFrame.reset(new AX25Frame(
                AX25Header::Ptr( new AX25Header(
                    currentAX25inputFrame->GetHeader()->Destination(),
                    currentAX25inputFrame->GetHeader()->Source(),
                    currentAX25inputFrame->GetHeader()->DigisReversed())),
                AX25Payload::Ptr(new AX25_DM(false))));
            output();
        }
        task = 0;
        return;
    case 7: // DL_DISCONNECT_Request
        // DL_DISCONNECT_Confirm
        task = 0;
        return;
    case 8: // Received DISC
        currentAX25outputFrame.reset(new AX25Frame(
            AX25Header::Ptr( new AX25Header(
                currentAX25inputFrame->GetHeader()->Destination(),
                currentAX25inputFrame->GetHeader()->Source(),
                currentAX25inputFrame->GetHeader()->DigisReversed())),
            AX25Payload::Ptr(new AX25_DM(true))));
        output();
        task = 0;
        return;
    case 9: // DL_UNIT_DATA_Request
        // DL_DISCONNECT_Confirm
        // UI_Command
        task = 0;
        return;
    case 13: // DL_CONNECT_Request
        sat = initial_sat;
        tiv = 2 * sat;
        establishDataLink();
        setLayer3Initiated();
        state = State::AwaitingConnection;
        task = 0;
        return;
    case 14: // Received SABM
        if (false) { // Not able to establish
            currentAX25outputFrame.reset(new AX25Frame(
                AX25Header::Ptr( new AX25Header(
                    currentAX25inputFrame->GetHeader()->Destination(),
                    currentAX25inputFrame->GetHeader()->Source(),
                    currentAX25inputFrame->GetHeader()->DigisReversed())),
                AX25Payload::Ptr(new AX25_DM(true))));
            output();
            task = 0;
            return;
        }
        task = 16;
        return;
    case 15: // Received SABME
        if (false) { // Not able to establish
            currentAX25outputFrame.reset(new AX25Frame(
                AX25Header::Ptr( new AX25Header(
                    currentAX25inputFrame->GetHeader()->Destination(),
                    currentAX25inputFrame->GetHeader()->Source(),
                    currentAX25inputFrame->GetHeader()->DigisReversed())),
                AX25Payload::Ptr(new AX25_DM(true))));
            output();
            task = 0;
            return;
        }
        task = 18;
        return;
    case 16:
        version = ax25modulo_t::MOD8;
        task = 17;
        return;
    case 17:
        currentAX25outputFrame.reset(new AX25Frame(
            AX25Header::Ptr( new AX25Header(
                currentAX25inputFrame->GetHeader()->Destination(),
                currentAX25inputFrame->GetHeader()->Source(),
                currentAX25inputFrame->GetHeader()->DigisReversed())),
            AX25Payload::Ptr(new AX25_UA(false))));
        output();
        clearExceptionConditions();
        vs = va = vr = 0;
        // DL_CONNECT_Indication
        srt = initial_srt;
        tiv = 2 * srt;
        t3.start();
        state = State::Connected;
        task = 0;
    case 18:
        version = ax25modulo_t::MOD128;
        task = 17;
        return;
    default: // All other commands
        currentAX25outputFrame.reset(new AX25Frame(
            AX25Header::Ptr( new AX25Header(
                currentAX25inputFrame->GetHeader()->Destination(),
                currentAX25inputFrame->GetHeader()->Source(),
                currentAX25inputFrame->GetHeader()->DigisReversed())),
            AX25Payload::Ptr(new AX25_DM(
                !currentAX25inputFrame->GetPayload()->get_PF()))));
        output();
        task = 0;
    }
}

} // end namespace AX25Automaton //
