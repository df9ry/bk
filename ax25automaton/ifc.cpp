#include "ifc.hpp"

#include <cassert>

using namespace std;

namespace AX25Automaton {

Automaton::Automaton()
{
    worker.reset(new thread([this] () { loop(); }));
}

Automaton::~Automaton()
{
    state = State::Exit;
}

void Automaton::loop()
{
    while (state != State::Exit) {
        gate.wait();
        while ((state != State::Exit) && !inputQueue.empty()) {
            auto [ primitive, expedited ] = inputQueue.get();
            switch (state) {
            case State::DataLinkDisconnected:
                dataLinkDisconnected(primitive, expedited);
                break;
            case State::Exit:
                return;
            default:
                assert(false);
            } // end switch(state) //
        } // end while //
    } // end while //
} // end loop //

void Automaton::input(AX25Base::AX25Frame::Ptr frame, bool expedited)
{
    if (!frame)
        return;
    inputQueue.put(AX25FramePrimitive(frame), expedited);
    gate.notify();
}

void Automaton::output()
{

}

} // namespace AX25Automaton
