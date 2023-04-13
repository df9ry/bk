#ifndef AX25AUTOMATON_IFC_HPP
#define AX25AUTOMATON_IFC_HPP

#include "state.hpp"
#include "primitive.hpp"
#include "ax25timer.hpp"

#include <bkbase/semaphore.hpp>
#include <bkbase/queue2.hpp>
#include <ax25base/ax25frame.hpp>

#include <memory>
#include <thread>
#include <atomic>

namespace AX25Automaton {

class Automaton {
public:
    typedef std::shared_ptr<Automaton> Ptr_t;

    Automaton();
    ~Automaton();

    void input(AX25Base::AX25Frame::Ptr frame, bool expedited = false);

private:
    void loop();

    void dataLinkDisconnected(AbstractPrimitive& primitive, bool expedited);

    const int initial_srt{1000};
    const int initial_sat{1000};

    std::unique_ptr<std::thread> worker{nullptr};
    std::atomic<State> state{State::DataLinkDisconnected};
    semaphore gate{};
    Queue2<AbstractPrimitive> inputQueue{};
    AX25Base::AX25Frame::Ptr currentAX25inputFrame{nullptr};
    AX25Base::AX25Frame::Ptr currentAX25outputFrame{nullptr};
    bool inputIsExpedited{false};
    int task{0};
    AX25Base::ax25modulo_t version{AX25Base::ax25modulo_t::MOD8};
    int vs{0}, va{0}, vr{0};
    int srt{0}, sat{0}, tiv{0};
    AX25Timer t3{};

    void exec();
    void UI_Check();
    void clearExceptionConditions();
    void establishDataLink();
    void setLayer3Initiated();
    void output();
};

} // namespace AX25Automaton

#endif // AX25AUTOMATON_IFC_HPP
